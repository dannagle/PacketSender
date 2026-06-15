//
// Created by Tomas Gallucci on 4/26/26.
//

#include <stdexcept>
#include <QAbstractSocket>
#include "basetcpthread.h"

#include <QSslKey>

#include "fileutils.h"
#include "settingnames.h"

void BaseTcpThread::debugSocketState() const
{
    qDebug() << "=== BaseTcpThread::debugSocketState ===";
    qDebug() << "socketInterface:" << (socketInterface ? "PRESENT" : "NULL");
    qDebug() << "getSocketInterface() returns:" << getSocketInterface();
}

BaseTcpThread::BaseTcpThread(PacketSenderQSslSocketInterface* socketInterface,
                             QObject* parent)
    : QThread(parent)
    , socketInterface(socketInterface)
{
    if (!socketInterface) {
        throw std::invalid_argument("BaseTcpThread: socketInterface cannot be null");
    }

    // Set Qt parent on the underlying real socket for proper cleanup
    if (QSslSocket* realSocket = socketInterface->rawSocket()) {
        realSocket->setParent(this);
    }
}

BaseTcpThread::~BaseTcpThread()
{
    // Qt's parent-child system will clean up the socket
    // No explicit deleteLater() needed here
}

void BaseTcpThread::stop()
{
    requestInterruption();
}

bool BaseTcpThread::shouldStop() const
{
    return isInterruptionRequested();
}

bool BaseTcpThread::interruptibleWaitForReadyRead(const int timeoutMs)
{
    const int chunk = 50;  // check every 50 ms
    int remaining = timeoutMs;

    QDEBUG() << "initial remaining: " << remaining;

    while (remaining > 0 && !shouldStop()) {
        if (getSocketInterface() && getSocketInterface()->waitForReadyRead(chunk)) {
            QDEBUG() << "inside if waitForReadyRead(chunk)";
            return true;
        }
        remaining -= chunk;
        QDEBUG() << "remaining after substraction: " << remaining;
        msleep(1);  // tiny yield
    }

    return false;
}

bool BaseTcpThread::isValid() const
{
    return isSocketValid();
}

bool BaseTcpThread::isConnected() const
{
    auto* s = getSocketInterface();
    if (!s) {
        return false;
    }
    return s->getSocketState() == QAbstractSocket::ConnectedState;
}

PacketSenderQSslSocketInterface* BaseTcpThread::getSocketInterface() const
{
    // qDebug() << "=== getSocket() called ===";
    // qDebug() << "  socketInterface:" << (socketInterface ? "YES" : "NULL");
    // qDebug() << "  socket (unique_ptr):" << (socket ? "YES" : "NULL");
    // qDebug() << "  socketInterface? socketInterface->rawSocket() : nullptr: " << ((socketInterface? socketInterface->rawSocket() : nullptr) ? "NOT NULL" : "NULL");

    return socketInterface.get();
}

QAbstractSocket::SocketState BaseTcpThread::getSocketState() const
{
    return socketInterface? socketInterface->getSocketState() : QAbstractSocket::UnconnectedState;
}

QByteArray BaseTcpThread::readSocketData()
{
    return socketInterface ? socketInterface->readData() : "";
}

bool BaseTcpThread::isSocketEncrypted() const
{
    return socketInterface ? socketInterface->isEncrypted() : false;
}

quint16 BaseTcpThread::getPeerPort() const
{
    return socketInterface ? socketInterface->getPeerPort() : 0;
}

quint16 BaseTcpThread::getLocalPort() const
{
    return socketInterface ? socketInterface->getLocalPort() : 0;
}

void BaseTcpThread::sleep(unsigned long usec)
{
    usleep(usec);
}

QHostAddress BaseTcpThread::getSocketPeerAddress() const
{
    return socketInterface ? socketInterface->getPeerAddress() : QHostAddress();
}

bool BaseTcpThread::isSocketValid() const
{
    // in production, socketInterface shouldn't be null.
    // But we can set socketInterface to null in the unit tests
    // and there might be a rare instance where it could become null in production
    return socketInterface && socketInterface->isValid();
}

QAbstractSocket::NetworkLayerProtocol BaseTcpThread::getIPConnectionProtocol() const
{
    qWarning() << "does this go bang if we try to do !getSocketInterface(): " << !getSocketInterface();

    if (!getSocketInterface()) {
        qWarning() << "getIPConnectionProtocol() called on BaseTcpThread with null socket";
        return QAbstractSocket::IPv4Protocol;   // safe default
    }

    QHostAddress peerAddr = getSocketPeerAddress();
    if (peerAddr.isNull() || peerAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol) {
        // Socket exists but is not yet connected (or connection failed)
        qWarning() << "getIPConnectionProtocol() called but peerAddress is not valid";
        return QAbstractSocket::IPv4Protocol;   // safe default
    }

    return peerAddr.protocol();
}

void BaseTcpThread::sendOutgoingPacket(Packet& packet)
{
    PacketSenderQSslSocketInterface* psSocketInterface = getSocketInterface();
    if (!psSocketInterface) {
        qWarning() << "sendOutgoingPacket: No socket available";
        emit connectionStatus("Error: No socket available");
        emit error(QAbstractSocket::SocketAccessError);
        return;
    }

    if (getSocketState() != QAbstractSocket::ConnectedState) {
        qWarning() << "sendOutgoingPacket: Socket is not connected (state =" << getSocketState() << ")";
        emit connectionStatus("Error: Socket not connected");
        emit error(QAbstractSocket::SocketAccessError);
        return;
    }

    QString errorMsg;
    if (!(packet.isValidForSending(&errorMsg)))
    {
        qDebug() << "=== VALIDATION FAILED ===";
        qDebug() << "Error message from isValidForSending:" << errorMsg;
        qDebug() << "Packet hexString was:" << packet.hexString;
        qDebug() << "Packet toIP was:" << packet.toIP;
        qDebug() << "Packet port was:" << packet.port;

        emit errorMessage(errorMsg);
        return;
    }

    emit connectionStatus("Sending data: " + packet.asciiString());

    psSocketInterface->write(packet.getByteArray());
    emit packetSent(packet);
}

void BaseTcpThread::closeConnection()
{
    const auto s = getSocketInterface();
    if (s)
    {
        if (getSocketState() == QAbstractSocket::ConnectedState ||
            getSocketState() == QAbstractSocket::ClosingState) {
            QDEBUG() << "got inside if statement that calls disconnectFromHost()";
            s->disconnectFromHost();
            s->waitForDisconnected(500);  // shorter timeout is fine here
            }

        s->close();

        emit connectionStatus("Disconnected");
        QDEBUG() << "Single packet sent. Disconnected.";
    }
}

QString BaseTcpThread::getPeerAddressAsString() const
{
    qDebug() << "getPeerAddressAsString() called";
    if (!getSocketInterface()) {
        qDebug() << "  → No socket, returning empty string";
        return "";
    }

    QHostAddress addr = getSocketPeerAddress();
    if (addr.isNull()) {
        qDebug() << "  → Null peer address, returning empty string";
        return "";
    }

    QAbstractSocket::NetworkLayerProtocol protocol = addr.protocol();

    if (protocol == QAbstractSocket::IPv6Protocol) {
        // Only strip IPv4-mapped addresses. Leave real IPv6 addresses alone.
        QString result = Packet::removeIPv6Mapping(addr);
        qDebug() << "  IPv6 result =" << result;
        return result;
    } else {
        QString result = addr.toString();
        qDebug() << "  IPv4 result =" << result;
        return result;
    }
}

void  BaseTcpThread::loadSnakeOilCertificate()
{
    const QByteArray decoded = FileUtils::decodeBase64EncodedResourceFile(SNAKEOIL_BASE64_CERT);
    const QSslCertificate certificate(decoded, QSsl::Pem);

    if (!certificate.isNull() && getSocketInterface()) {
        getSocketInterface()->setLocalCertificate(certificate);
    }
}

void BaseTcpThread::loadSnakeOilKey()
{
    const QByteArray decoded = FileUtils::decodeBase64EncodedResourceFile(SNAKEOIL_BASE64_KEY);
    const QSslKey sslKey(decoded, QSsl::Rsa, QSsl::Pem);

    if (!sslKey.isNull() && getSocketInterface()) {
        getSocketInterface()->setPrivateKey(sslKey);
    }
}

void BaseTcpThread::loadSnakeOilCerts()
{
    // Certificate
    loadSnakeOilCertificate();

    // Private Key
    loadSnakeOilKey();
}

void BaseTcpThread::loadSSLCerts(bool allowSnakeOil)
{
    auto sock = getSocketInterface();

    if (!sock) {
        emit errorMessage("loadSSLCerts called with null socketInterface");
        return;
    }

    if (allowSnakeOil)
    {
        loadSnakeOilCerts();
        return;
    }

    const QSettings& settings = getSettings();

    // Production / user-provided certs
    QString certPath = settings.value(SET_LOCAL_CERTIFICATE_PATH).toString();
    QString keyPath  = settings.value(SSL_PRIVATE_KEY_PATH).toString();

    if (!certPath.isEmpty()) {
        sock->setLocalCertificate(certPath);

        if (!sock->hasLocalCertificate()) {
            emit errorMessage("SSL: Failed to load certificate from: " + certPath);
        }
    }
    if (!keyPath.isEmpty()) {
        sock->setPrivateKey(keyPath);

        if (!sock->hasPrivateKey()) {
            emit errorMessage("SSL: Failed to load private key from: " + keyPath);
        }
    }
}
