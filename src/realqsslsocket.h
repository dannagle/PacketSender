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

    [[nodiscard]] bool isEncrypted() const override;
    void ignoreSslErrors() override;
    void disconnectFromHost() override;

    [[nodiscard]] quint16 getPeerPort() const override;
    [[nodiscard]] quint16 getLocalPort() const override;
    [[nodiscard]] QHostAddress getPeerAddress() const override;

    [[nodiscard]] QByteArray readData() const override;
    [[nodiscard]] QAbstractSocket::SocketState getSocketState() const override;

    [[nodiscard]] QSslSocket* rawSocket() const override { return socket; }

private:
    QSslSocket* socket = nullptr;
};


#endif //REALQSSLSOCKET_H
