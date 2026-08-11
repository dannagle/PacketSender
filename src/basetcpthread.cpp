//
// Created by Tomas Gallucci on 4/26/26.
//

#include <stdexcept>
#include <QAbstractSocket>
#include "basetcpthread.h"

#include <QSslKey>
#include <QUuid>

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

    assignUniqueId();
}

BaseTcpThread::~BaseTcpThread()
{
    if (isThreadRunning())
    {
        shutdown();   // stop and clean up
    } else
    {
        // else already stopped - just clean up socket
        closeConnection();
    }
}

void BaseTcpThread::shutdown()
{
    if (isThreadRunning())
        QDEBUG() << "We were running. Stopping.";
    {
        acceptingSends = false;
        threadState = ThreadState::Stopping;

        stop();                    // Use our own stop abstraction
        quit();                    // Ask event loop to exit
        bool finished = wait(2500); // Slightly longer timeout is safer

        if (!finished)
        {
            qWarning() << metaObject()->className() << "did not stop cleanly within timeout";
            terminate();           // Last resort
            wait(2500); // Slightly longer timeout is safer
        }
        threadState = ThreadState::Stopped;
    }

    {
        QMutexLocker lock(&sendQueueMutex);
        sendQueue.clear();
    }

    closeConnection();             // Always try to clean up socket
}

bool BaseTcpThread::isThreadRunning() const
{
    return threadState.load() == ThreadState::Running || threadState.load() == ThreadState::Stopping;
}

QString BaseTcpThread::getThreadStateAsString() const
{
    switch (threadState.load())
    {
        case ThreadState::Created: return QStringLiteral("Created");
        case ThreadState::Running: return QStringLiteral("Running");
        case ThreadState::Stopping: return QStringLiteral("Stopping");
        case ThreadState::Stopped: return QStringLiteral("Stopped");
        case ThreadState::Error: return QStringLiteral("Error");
    }
}

void BaseTcpThread::stop()
{
    if (isThreadRunning())
    {
        QDEBUG() << "Thread was running";
        requestInterruption();
    } else
    {
        QDEBUG() << "Thread was not running";
    }
}

bool BaseTcpThread::shouldStop() const
{
    return isInterruptionRequested();
}

bool BaseTcpThread::isInterruptionRequested() const
{
    return QThread::isInterruptionRequested();
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

QString BaseTcpThread::id() const
{
    return id_.has_value() ? id_.value() : QString();
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

void BaseTcpThread::enqueuePacket(const Packet& packet)
{
    if (!acceptingSends.load())
        return;

    QMutexLocker lock(&sendQueueMutex);
    sendQueue.enqueue(packet);
}

void BaseTcpThread::drainSendQueue()
{

    QMutexLocker lock(&sendQueueMutex);

    if (sendQueue.isEmpty())
        return;

    while (!sendQueue.isEmpty()) {
        Packet p = sendQueue.dequeue();
        lock.unlock();
        sendOutgoingPacket(p);   // existing method
        lock.relock();
    }
}

QAbstractSocket::SocketState BaseTcpThread::getSocketState() const
{
    return socketInterface? socketInterface->getSocketState() : QAbstractSocket::UnconnectedState;
}

QByteArray BaseTcpThread::readSocketData()
{
    return socketInterface ? socketInterface->readData() : "";
}

bool BaseTcpThread::isValidForSending(Packet& packet, QString* errorMessage = nullptr) const
{
    if (packet.port == 0) {
        if (errorMessage) *errorMessage = "Port must be a positive number";
        return false;
    }

    if (packet.getByteArray().isEmpty()) {
        if (errorMessage) *errorMessage = "No data to send (hexString is empty)";
        return false;
    }

    return true;
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

    if (QString errorMsg; !(isValidForSending(packet, &errorMsg)))
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
    if (!s) return;

    bool shouldEmit = false;
    if (getSocketState() == QAbstractSocket::ConnectedState ||
        getSocketState() == QAbstractSocket::ClosingState) {
            QDEBUG() << "got inside if statement that calls disconnectFromHost()";
            s->disconnectFromHost();
            s->waitForDisconnected(500);  // shorter timeout is fine here
            shouldEmit = true;
        }

    s->close();

    QDEBUG() << "shouldEmit in BaseTcpThread::closeConnection: " << shouldEmit;

    if (shouldEmit)
    {
        emit connectionStatus("Disconnected");
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

void BaseTcpThread::assignUniqueId()
{
    if (id_.has_value())
    {
        throw std::runtime_error("unique id is already set for Connection object");
    }

    id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}
