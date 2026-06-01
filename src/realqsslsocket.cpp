//
// Created by Tomas Gallucci on 5/31/26.
//

#include "realqsslsocket.h"
#include <QSslSocket>

RealQSslSocket::RealQSslSocket(QSslSocket* socket)
    : socket(socket)
{
}

QList<QSslError> RealQSslSocket::sslHandshakeErrors() const
{
    if (!socket) return {};
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return socket->sslErrors();
#else
    return socket->sslHandshakeErrors();
#endif
}

QSslCipher RealQSslSocket::sessionCipher() const
{
    return socket ? socket->sessionCipher() : QSslCipher();
}

QSslCertificate RealQSslSocket::peerCertificate() const
{
    return socket ? socket->peerCertificate() : QSslCertificate();
}

void RealQSslSocket::connectToHost(const QString& hostName, quint16 port,
                                   QIODevice::OpenMode openMode,
                                   QAbstractSocket::NetworkLayerProtocol protocol)
{
    if (socket) socket->connectToHost(hostName, port, openMode, protocol);
}

void RealQSslSocket::connectToHostEncrypted(const QString& hostName, quint16 port,
                                            QIODevice::OpenMode openMode,
                                            QAbstractSocket::NetworkLayerProtocol protocol)
{
    if (socket) socket->connectToHostEncrypted(hostName, port, openMode, protocol);
}

bool RealQSslSocket::isValid() const
{
    return socket ? socket->isValid() : false;
}

bool RealQSslSocket::waitForConnected(int msecs)
{
    return socket ? socket->waitForConnected(msecs) : false;
}

bool RealQSslSocket::waitForEncrypted(int msecs)
{
    return socket ? socket->waitForEncrypted(msecs) : false;
}

bool RealQSslSocket::isEncrypted() const
{
    return socket ? socket->isEncrypted() : false;
}

void RealQSslSocket::ignoreSslErrors()
{
    if (socket) socket->ignoreSslErrors();
}

void RealQSslSocket::disconnectFromHost()
{
    if (socket) socket->disconnectFromHost();
}

quint16 RealQSslSocket::getPeerPort() const
{
    return socket ? socket->peerPort() : 0;
}

quint16 RealQSslSocket::getLocalPort() const
{
    return socket ? socket->localPort() : 0;
}

QHostAddress RealQSslSocket::getPeerAddress() const
{
    return socket ? socket->peerAddress() : QHostAddress();
}

QByteArray RealQSslSocket::readData() const
{
    return socket ? socket->readAll() : QByteArray();
}

QAbstractSocket::SocketState RealQSslSocket::getSocketState() const
{
    return socket ? socket->state() : QAbstractSocket::SocketState::UnconnectedState;
}
