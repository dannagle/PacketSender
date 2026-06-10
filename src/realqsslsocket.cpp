//
// Created by Tomas Gallucci on 5/31/26.
//

#include "realqsslsocket.h"

#include <QSslKey>
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

bool RealQSslSocket::waitForReadyRead(int msecs)
{
    return socket ? socket->waitForReadyRead(msecs) : false;
}

bool RealQSslSocket::waitForDisconnected(int msecs)
{
    return socket->waitForDisconnected(msecs);
}

qint64 RealQSslSocket::write(const QByteArray& data)
{
    return socket->write(data);
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

qint64 RealQSslSocket::bytesAvailable() const
{
    return socket->bytesAvailable();
}

QString RealQSslSocket::getErrorString() const
{
    return socket->errorString();
}

void RealQSslSocket::setLocalCertificate(const QString &fileName,
                                         QSsl::EncodingFormat format)
{
    if (socket) {
        socket->setLocalCertificate(fileName, format);
    }
}

void RealQSslSocket::setLocalCertificate(const QSslCertificate &certificate)
{
    if (socket) {
        socket->setLocalCertificate(certificate);
    }
}

void RealQSslSocket::setPrivateKey(const QString &fileName,
                                   QSsl::KeyAlgorithm algorithm,
                                   QSsl::EncodingFormat format,
                                   const QByteArray &passPhrase)
{
    if (socket) {
        socket->setPrivateKey(fileName, algorithm, format, passPhrase);
    }
}

void RealQSslSocket::setPrivateKey(const QSslKey& key)
{
    socket->setPrivateKey(key);
}

bool RealQSslSocket::hasLocalCertificate() const
{
    return !rawSocket()->localCertificate().isNull();
}

QSslCertificate RealQSslSocket::localCertificate() const
{
    return socket->localCertificate();
}

bool RealQSslSocket::hasPrivateKey() const
{
    return !rawSocket()->privateKey().isNull();
}

void RealQSslSocket::setProtocol(QSsl::SslProtocol protocol)
{
    socket->setProtocol(protocol);
}

int RealQSslSocket::getSocketDescriptor() const
{
    return static_cast<int>(socket->socketDescriptor());
}

bool RealQSslSocket::setSocketDescriptor(qintptr socketDescriptor, QAbstractSocket::SocketState state,
                                         QIODeviceBase::OpenMode openMode)
{
    return socket->setSocketDescriptor(socketDescriptor, state, openMode);
}

void RealQSslSocket::close()
{
    socket->close();
}
