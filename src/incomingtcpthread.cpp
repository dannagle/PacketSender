//
// Created by Tomas Gallucci on 6/8/26.
//

#include "incomingtcpthread.h"
#include "basetcpthread.h"
#include "ConnectionStatusMessages.h"
#include "realqsslsocket.h"
#include "settingnames.h"

PacketSenderQSslSocketInterface* IncomingTcpThread::createSocketWithDescriptor(int socketDescriptor)
{

    qDebug() << "socketDescriptor in helper method: " << socketDescriptor;

    auto realSocket = std::make_unique<RealQSslSocket>(new QSslSocket());

    // IMPORTANT: Set the descriptor BEFORE passing to base
    bool ok = realSocket->setSocketDescriptor(
        socketDescriptor,
        QAbstractSocket::ConnectedState,   // the socket is already connected
        QIODevice::ReadWrite
    );

    if (!ok) {
        // handle error - e.g. throw or log
        qDebug() << "setSocketDescriptor" << socketDescriptor
         << "ok? " << ok << "\n"
         << "error:" << realSocket->rawSocket()->errorString();
        throw std::runtime_error("Failed to set socket descriptor on QSslSocket");
    }

    return realSocket.release();
}

IncomingTcpThread::IncomingTcpThread(PacketSenderQSslSocketInterface* socketInterface,
                                     bool isSecure,
                                     bool isPersistent,
                                     QObject* parent)
    : BaseTcpThread(socketInterface, parent)
{
    if (socketInterface->getSocketDescriptor() == 0)
    {
        QDEBUG() << "before throwing: socket descriptor is 0";
        throw std::runtime_error("Socket descriptor is zero");
    }

    shouldUseSSL = isSecure;
    persistent = isPersistent;

    // as much as we may want to emit `ConnectionStatus()` here, we can't because the signals
    // don't get connected to the slots until after this constructor runs.
}

IncomingTcpThread::IncomingTcpThread(int socketDescriptor,
                                     bool isSecure,
                                     bool isPersistent,
                                     QObject* parent)
    : IncomingTcpThread(createSocketWithDescriptor(socketDescriptor),
                        isSecure,
                        isPersistent,
                        parent)
{
}

IncomingTcpThread::~IncomingTcpThread() = default;

Packet IncomingTcpThread::buildReceivedPacket()
{
    Packet p;
    p.timestamp = QDateTime::currentDateTime();
    p.name = p.timestamp.toString(DATETIMEFORMAT);
    p.tcpOrUdp = shouldUseSSL ? "SSL" : "TCP";
    p.fromIP = getPeerAddressAsString();
    p.toIP = "You";
    p.port = getLocalPort();
    p.fromPort = getPeerPort();

    auto* sock = getSocketInterface();
    if (sock && sock->isValid() && (sock->getSocketState() == QAbstractSocket::ConnectedState) && !hasSslError) {
        sock->waitForReadyRead(3000);           // reasonable timeout for first data

        if (!shouldUseSSL)
        {
            emit connectionStatus(ConnectionStatusMessages::INCOMING_CONNECTION_ACCEPTED());
        }

        QByteArray data = readSocketData();     // assuming this helper exists in Base
        p.hexString = Packet::byteArrayToHex(data);
        p.asciiString();                        // ensure ascii field is populated
    } else
    {
        // we probably can't get here because the socket would have been validated in the constructor
        // however, in an abundance of caution, it's possible we could have had a valid connection
        // but not be connected to the socket despite the fact we set the socketDescriptor in the constructor
        // which had to return true indicating a successful connection to the socket. "Anyway,"
        emit connectionStatus(ConnectionStatusMessages::ERROR_SOCKET_NOT_CONNECTED());
    }

    return p;
}

void IncomingTcpThread::sendSmartReplyIfConfigured(const Packet& receivedPacket)
{
    const QSettings& settings = getSettings();

    const bool sendResponseEnabled = settings.value(SEND_RESPONSE, false).toBool();
    const QString responseHex = settings.value(RESPONSE_HEX, "").toString().trimmed();

    QDEBUG() << "\nsendResponseEnabled: " << sendResponseEnabled << "\n";
    QDEBUG() << "\nresponseHex: " << responseHex << "\n";

    if (!sendResponseEnabled || responseHex.isEmpty()) {
        return;
    }

    Packet reply = receivedPacket;           // copy metadata (timestamp, IPs, ports, etc.)
    reply.hexString = responseHex;

    QString expanded = Packet::macroSwap(reply.asciiString());
    reply.hexString = Packet::ASCIITohex(expanded);

    QDEBUG() << "about to call sendOutgoingPacket";
    sendOutgoingPacket(reply);
}

void IncomingTcpThread::emitSSLDiagnosticPackets()
{
    auto* sock = getSocketInterface();
    if (!sock || !sock->isEncrypted()) {
        return;
    }

    Packet info;
    info.timestamp = QDateTime::currentDateTime();
    info.name = info.timestamp.toString(DATETIMEFORMAT);
    info.toIP = "You";
    info.port = getLocalPort();
    info.fromIP = getPeerAddressAsString();
    info.fromPort = getPeerPort();
    info.tcpOrUdp = "SSL";

    // Cipher info
    QSslCipher cipher = sock->sessionCipher();
    info.errorString = "Encrypted with " + cipher.encryptionMethod();
    emit packetSent(info);

    info.errorString = "Authenticated with " + cipher.authenticationMethod();
    emit packetSent(info);

    // Certificate info
    info.errorString = "Peer cert issued by " +
        sock->peerCertificate().issuerInfo(QSslCertificate::CommonName).join("\n");
    emit packetSent(info);

    info.errorString = "Our Cert issued by " +
        sock->localCertificate().issuerInfo(QSslCertificate::CommonName).join("\n");
    emit packetSent(info);
}

void IncomingTcpThread::performSSLHandshakeIfNeeded()
{
    if (!shouldUseSSL)
    {
        return;
    }

    auto* sock = getSocketInterface();
    if (!sock) {
        emit errorMessage("performSSLHandshakeIfNeeded: null socket");
        return;
    }

    const QSettings& settings = getSettings();
    const bool useSnakeOil = settings.value(LOAD_SNAKEOIL_CERTS, true).toBool();   // "serverSnakeOilCheck"

    // === 1. Load certificates (reuse logic from OutgoingTcpThread) ===
    if (useSnakeOil) {
        loadSnakeOilCerts();
    } else {
        loadSSLCerts(useSnakeOil);
    }

    // === 2. Standard SSL setup ===
    sock->setProtocol(QSsl::AnyProtocol);

    if (settings.value(IGNORE_SSL_CHECK, true).toBool()) {
        sock->ignoreSslErrors();
    }

    // === 3. Start server-side encryption ===
    sock->startServerEncryption();

    if (!sock->waitForEncrypted(5000)) {
        QDEBUG() << "emitting SSL Error message in IncomingTcpThread";
        emit connectionStatus(ConnectionStatusMessages::SSL_HANDSHAKE_FAILED());
        hasSslError = true;
        emit errorMessage("Incoming SSL handshake failed (waitForEncrypted timeout)");
        return;
    }

    emit connectionStatus(ConnectionStatusMessages::SSL_CONNECTED());

    // === 4. Emit diagnostic packets (this is the part that was in old TCPThread) ===
    emitSSLDiagnosticPackets();
}

void IncomingTcpThread::persistentConnectionLoop()
{
    auto* sock = getSocketInterface();
    if (!sock) return;

    while (!shouldStop())
    {
        if (sock->waitForReadyRead(500))
        {
            const Packet received = buildReceivedPacket();
            if (!received.hexString.isEmpty())
            {
                emit packetReceived(received);
                sendSmartReplyIfConfigured(received);
            }
        }

        QThread::msleep(10);   // prevent tight CPU loop
    }
}

void IncomingTcpThread::handleIncomingConnection()
{
    auto* sock = getSocketInterface();
    if (!sock) {
        emit errorMessage("handleIncomingConnection: null socket interface");
        return;
    }

    performSSLHandshakeIfNeeded();

    const Packet received = buildReceivedPacket();
    emit packetReceived(received);

    sendSmartReplyIfConfigured(received);
}

void IncomingTcpThread::run()
{
    if (!getSocketInterface()) {
        emit errorMessage("IncomingTcpThread: no socket interface");
        return;
    }

    handleIncomingConnection();

    if (persistent)
    {
        persistentConnectionLoop();
    }

    closeConnection();
}
