//
// Created by Tomas Gallucci on 4/26/26.
//

#include <stdexcept>
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

bool BaseTcpThread::isValid() const
{
    return socket != nullptr && socket->isValid();
}

QSslSocket* BaseTcpThread::getSocket() const
{
    return socket;
}

bool BaseTcpThread::isSocketEncrypted() const
{
    return socket ? socket->isEncrypted() : false;
}

quint16 BaseTcpThread::getPeerPort() const
{
    return socket ? socket->peerPort() : 0;
}

quint16 BaseTcpThread::getLocalPort() const
{
    return socket ? socket->localPort() : 0;
}

QHostAddress BaseTcpThread::getSocketPeerAddress() const
{
    return socket ? socket->peerAddress() : QHostAddress();
}

QAbstractSocket::NetworkLayerProtocol BaseTcpThread::getIPConnectionProtocol() const
{
    if (!socket) {
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

QString BaseTcpThread::getPeerAddressAsString() const
{
    qDebug() << "getPeerAddressAsString() called";
    if (!socket) {
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

