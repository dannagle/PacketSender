//
// Created by Tomas Gallucci on 4/26/26.
//

#include <stdexcept>
#include <QAbstractSocket>
#include "basetcpthread.h"


BaseTcpThread::BaseTcpThread(QSslSocket* socket, QObject* parent)
    : QThread(parent),
      socket(socket)
{
    if (!socket) {
        throw std::invalid_argument("BaseTcpThread: socket cannot be null");
    }

    // Have Qt automagically clean up the thread when BaseTcpThread is destructed
    socket->setParent(this);
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
        if (getSocket() && getSocket()->waitForReadyRead(chunk)) {
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

QSslSocket* BaseTcpThread::getSocket() const
{
    return socket.get();
}

QAbstractSocket::SocketState BaseTcpThread::getSocketState() const
{
    return socket? socket->state() : QAbstractSocket::UnconnectedState;
}

bool BaseTcpThread::isSocketEncrypted() const
{
    return getSocket() ? socket->isEncrypted() : false;
}

quint16 BaseTcpThread::getPeerPort() const
{
    return getSocket() ? socket->peerPort() : 0;
}

quint16 BaseTcpThread::getLocalPort() const
{
    return getSocket() ? socket->localPort() : 0;
}

void BaseTcpThread::sleep(unsigned long usec)
{
    usleep(usec);
}

QHostAddress BaseTcpThread::getSocketPeerAddress() const
{
    return getSocket() ? socket->peerAddress() : QHostAddress();
}

bool BaseTcpThread::isSocketValid() const
{
    return getSocket() != nullptr && socket->isValid();
}

QAbstractSocket::NetworkLayerProtocol BaseTcpThread::getIPConnectionProtocol() const
{
    if (!getSocket()) {
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
    QSslSocket* sock = getSocket();
    if (!sock) {
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

    sock->write(packet.getByteArray());
    emit packetSent(packet);
}

QString BaseTcpThread::getPeerAddressAsString() const
{
    qDebug() << "getPeerAddressAsString() called";
    if (!getSocket()) {
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
