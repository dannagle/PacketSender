//
// Created by Tomas Gallucci on 5/31/26.
//

#ifndef REALQSSLSOCKET_H
#define REALQSSLSOCKET_H
#include "packetsenderqsslsocketinterface.h"


class QSslSocket;

class RealQSslSocket : public PacketSenderQSslSocketInterface
{
public:
    explicit RealQSslSocket(QSslSocket* socket);

    QList<QSslError> sslHandshakeErrors() const override;
    QSslCipher sessionCipher() const override;
    QSslCertificate peerCertificate() const override;

    void connectToHost(const QString& hostName, quint16 port,
                       QIODevice::OpenMode openMode = QIODevice::ReadWrite,
                       QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol) override;

    void connectToHostEncrypted(const QString& hostName, quint16 port,
                                QIODevice::OpenMode openMode = QIODevice::ReadWrite,
                                QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol) override;

    [[nodiscard]] bool isValid() const override;

    bool waitForConnected(int msecs) override;
    bool waitForEncrypted(int msecs) override;
    bool waitForReadyRead(int msecs) override;
    bool waitForDisconnected(int msecs) override;

    qint64 write(const QByteArray &data) override;

    [[nodiscard]] bool isEncrypted() const override;
    void ignoreSslErrors() override;
    void disconnectFromHost() override;

    [[nodiscard]] quint16 getPeerPort() const override;
    [[nodiscard]] quint16 getLocalPort() const override;
    [[nodiscard]] QHostAddress getPeerAddress() const override;

    [[nodiscard]] QByteArray readData() const override;
    [[nodiscard]] QAbstractSocket::SocketState getSocketState() const override;

    qint64 bytesAvailable() const override;
    QString getErrorString() const override;

    void setLocalCertificate(const QString &fileName, QSsl::EncodingFormat format = QSsl::Pem) override;
    void setLocalCertificate(const QSslCertificate& certificate) override;
    void setPrivateKey(const QString& fileName, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format,
                       const QByteArray& passPhrase) override;
    void setPrivateKey(const QSslKey &key) override;
    bool hasLocalCertificate() const override;
    QSslCertificate localCertificate() const override;
    bool hasPrivateKey() const override;
    void setProtocol(QSsl::SslProtocol protocol) override;

    [[nodiscard]] int getSocketDescriptor() const override;
    bool setSocketDescriptor(
        qintptr socketDescriptor,
        QAbstractSocket::SocketState state = QAbstractSocket::ConnectedState,
        QIODeviceBase::OpenMode openMode = QIODeviceBase::ReadWrite) override;

    [[nodiscard]] QSslSocket* rawSocket() const override { return socket; }

    void close() override;

private:
    QSslSocket* socket = nullptr;
};


#endif //REALQSSLSOCKET_H
