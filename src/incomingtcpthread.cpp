//
// Created by Tomas Gallucci on 6/8/26.
//

#include "incomingtcpthread.h"
#include "basetcpthread.h"
#include "realqsslsocket.h"
#include "settingnames.h"

PacketSenderQSslSocketInterface* IncomingTcpThread::createSocketWithDescriptor(int socketDescriptor)
{

    qDebug() << "socketDescriptor in helper method: " << socketDescriptor;

    auto realSocket = std::make_unique<RealQSslSocket>(new QSslSocket());

    // IMPORTANT: Set the descriptor BEFORE passing to base
    if (!realSocket->setSocketDescriptor(socketDescriptor)) {
        // handle error - e.g. throw or log
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
    shouldUseSSL = isSecure;
    persistent = isPersistent;
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
    if (sock && sock->isValid() && (sock->getSocketState() == QAbstractSocket::ConnectedState)) {
        sock->waitForReadyRead(3000);           // reasonable timeout for first data
        QByteArray data = readSocketData();     // assuming this helper exists in Base
        p.hexString = Packet::byteArrayToHex(data);
        p.asciiString();                        // ensure ascii field is populated
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
        emit errorMessage("Incoming SSL handshake failed (waitForEncrypted timeout)");
        return;
    }

    // === 4. Emit diagnostic packets (this is the part that was in old TCPThread) ===
    emitSSLDiagnosticPackets();
}

void IncomingTcpThread::handleIncomingConnection()
{
    auto* sock = getSocketInterface();
    if (!sock) {
        emit errorMessage("handleIncomingConnection: null socket interface");
        return;
    }

    emit connectionStatus("Incoming connection accepted");

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
    closeConnection();
}
