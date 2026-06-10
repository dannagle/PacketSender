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
    virtual bool waitForReadyRead(int msecs = 30000) = 0;
    virtual bool waitForDisconnected(int msecs = 30000) = 0;

    virtual qint64 write(const QByteArray &data) = 0;

    [[nodiscard]] virtual bool isEncrypted() const = 0;
    virtual void disconnectFromHost() = 0;

    [[nodiscard]] virtual bool isValid() const = 0;

    virtual quint16 getPeerPort() const = 0;
    virtual quint16 getLocalPort() const = 0;
    virtual QHostAddress getPeerAddress() const = 0;

    virtual QByteArray readData() const = 0;
    virtual qint64 bytesAvailable() const = 0;

    virtual QAbstractSocket::SocketState getSocketState() const = 0;
    virtual QString getErrorString() const = 0;

    virtual void setLocalCertificate(const QString &fileName,
                                 QSsl::EncodingFormat format = QSsl::Pem) = 0;

    virtual void setLocalCertificate(const QSslCertificate &certificate) = 0;

    virtual void setPrivateKey(const QString &fileName,
                               QSsl::KeyAlgorithm algorithm = QSsl::Rsa,
                               QSsl::EncodingFormat format = QSsl::Pem,
                               const QByteArray &passPhrase = QByteArray()) = 0;

    virtual void setPrivateKey(const QSslKey &key) = 0;
    virtual void setProtocol(QSsl::SslProtocol protocol) = 0;

    virtual bool hasLocalCertificate() const = 0;
    virtual bool hasPrivateKey() const = 0;

    [[nodiscard]] virtual int getSocketDescriptor() const = 0;   // or qintptr, depending on your code

    // NEW: setter - mirror Qt's signature for compatibility
    virtual bool setSocketDescriptor(qintptr socketDescriptor,
                                     QAbstractSocket::SocketState state = QAbstractSocket::ConnectedState,
                                     QIODeviceBase::OpenMode openMode = QIODeviceBase::ReadWrite) = 0;

    virtual QSslCertificate localCertificate() const = 0;

    // Helper for gradual migration
    [[nodiscard]] virtual QSslSocket* rawSocket() const { return nullptr; }

    virtual void close() = 0;

protected:
    PacketSenderQSslSocketInterface() = default;
};


#endif //PACKETSENDERQSSLSOCKETINTERFACE_H
