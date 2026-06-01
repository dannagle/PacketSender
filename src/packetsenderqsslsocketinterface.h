//
// Created by Tomas Gallucci on 5/31/26.
//

#ifndef PACKETSENDERQSSLSOCKETINTERFACE_H
#define PACKETSENDERQSSLSOCKETINTERFACE_H

#include <QList>
#include <QSslError>
#include <QSslCertificate>
#include <QSslCipher>
#include <QAbstractSocket>

class QSslSocket;

class PacketSenderQSslSocketInterface
{
public:
    virtual ~PacketSenderQSslSocketInterface() = default;

    // === SSL-specific methods ===
    [[nodiscard]] virtual QList<QSslError> sslHandshakeErrors() const = 0;
    [[nodiscard]] virtual QSslCipher sessionCipher() const = 0;
    [[nodiscard]] virtual QSslCertificate peerCertificate() const = 0;
    virtual void ignoreSslErrors() = 0;

    // === Connection methods ===
    virtual void connectToHost(const QString &hostName, quint16 port,
                                       QIODevice::OpenMode openMode = QIODevice::ReadWrite,
                                       QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol) = 0;
    virtual void connectToHostEncrypted(const QString &hostName, quint16 port,
                                        QIODevice::OpenMode mode = QIODevice::ReadWrite,
                                        QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol) = 0;

    virtual bool waitForConnected(int msecs = 30000) = 0;
    virtual bool waitForEncrypted(int msecs = 30000) = 0;

    [[nodiscard]] virtual bool isEncrypted() const = 0;
    virtual void disconnectFromHost() = 0;

    [[nodiscard]] virtual bool isValid() const = 0;

    virtual quint16 getPeerPort() const = 0;
    virtual quint16 getLocalPort() const = 0;
    virtual QHostAddress getPeerAddress() const = 0;

    virtual QByteArray readData() const = 0;

    virtual QAbstractSocket::SocketState getSocketState() const = 0;

    // Helper for gradual migration
    [[nodiscard]] virtual QSslSocket* rawSocket() const { return nullptr; }

protected:
    PacketSenderQSslSocketInterface() = default;
};


#endif //PACKETSENDERQSSLSOCKETINTERFACE_H
