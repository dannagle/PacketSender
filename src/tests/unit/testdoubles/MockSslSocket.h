//
// Created by Tomas Gallucci on 3/17/26.
//

#ifndef MOCKSSLSOCKET_H
#define MOCKSSLSOCKET_H

#include <QSslCipher>
#include <QObject>
#include <QSslSocket>

#include "../../packetsenderqsslsocketinterface.h"

// Mock QSslSocket for testing (or use a real one in a controlled way)
class MockSslSocket : public QSslSocket, public PacketSenderQSslSocketInterface {
    Q_OBJECT
public:
    explicit MockSslSocket(QObject *parent = nullptr)
        // Do NOT call QSslSocket(parent) here
        // Qt handles initialization internally via d-pointer
    {
        // Your init code if any
        setParent(parent);  // optional, but good practice
    }

    // FIX: Explicitly delete copy/move to match base class
    MockSslSocket(const MockSslSocket &) = delete;
    MockSslSocket& operator=(const MockSslSocket &) = delete;
    MockSslSocket(MockSslSocket &&) = delete;
    MockSslSocket& operator=(MockSslSocket &&) = delete;

    [[nodiscard]] QSslSocket* rawSocket() const override
    {
        return const_cast<QSslSocket*>(static_cast<const QSslSocket*>(this));
    }

    [[nodiscard]] QAbstractSocket::SocketState getSocketState() const override
    {
        qDebug() << "=== MOCK mockState() called → returning" << mockState;
        return mockState;
    }


    void setMockCipher(const QSslCipher &cipher) { mockCipher = cipher; }

    QSslCipher sessionCipher() const { return mockCipher; }

    bool makeWaitForConnectedReturnFalse = false;
    bool waitForConnected(int msecs = 30000) override { qDebug() << "=== MOCK waitForConnected called → returning" << mockConnected; return makeWaitForConnectedReturnFalse? false: mockConnected; }
    bool waitForEncrypted(int msecs = 30000) { qDebug() << "=== MOCK waitForEncrypted called → returning" << mockEncrypted; return mockEncrypted; }
    bool isEncrypted() const {qDebug() << "=== MOCK isEncrypted called → returning" << mockEncrypted; return mockEncrypted; }

    [[nodiscard]] bool isValid() const override { return isMockValid;}
    void setIsValid(bool isValid) { isMockValid = isValid; }

    QList<QSslError> sslErrors() const { return mockSslErrors; }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QList<QSslError> sslHandshakeErrors() const { return mockSslErrors; }
#endif

    // === PacketSenderQSslSocketInterface methods ===
    QSslCertificate peerCertificate() const override
    {
        return QSslSocket::peerCertificate();
    }

    void connectToHostEncrypted(const QString& hostName, quint16 port,
                                QIODevice::OpenMode openMode = QIODevice::ReadWrite,
                                QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol) override
    {

    }

    void ignoreSslErrors() override
    {
        qDebug("ignoreSslErrors called in MockSslSocket");
    }

    // Mock setters
    void setMockConnected(bool val)
    {
        mockConnected = val;

        if (mockConnected)
        {
            mockState = QAbstractSocket::ConnectedState;
        } else
        {
            mockState = QAbstractSocket::UnconnectedState;
        }
    }

    void setMockBytesAvailable(qint64 bytes) { mockBytesAvailable = bytes; }

    qint64 getMockBytesAvailable() const
    {
        qDebug() << "=== MOCK getMockBytesAvailable() called → returning" << mockBytesAvailable;
        return mockBytesAvailable;
    }

    qint64 bytesAvailable() const override
    {
        return mockReadData.size();
    }

    void setMockEncrypted(bool val) { mockEncrypted = val; }
    void setMockSslErrors(const QList<QSslError> &errors) { mockSslErrors = errors; }
    void setMockState(const QAbstractSocket::SocketState &state) { mockState = state; }

    void setMockPeerAddress(const QHostAddress &address) { mockPeerAddress = address; }

    [[nodiscard]] NetworkLayerProtocol getIPConnectionProtocol() const
    {
        if (mockPeerAddress.protocol() == IPv6Protocol) {
            return IPv6Protocol;
        }
        return IPv4Protocol;
    }

    void setMockReadData(const QByteArray &data) { mockReadData = data; }

    [[nodiscard]] QByteArray readData() const override
    {
        return mockReadData;
    }

    void setMockPeerPort(quint16 port) { mockPeerPort = port; }
    void setMockLocalPort(quint16 port) { mockLocalPort = port; }

    [[nodiscard]] quint16 getPeerPort() const override {
        qDebug() << "=== MOCK peerPort() called → returning" << mockPeerPort;
        return mockPeerPort;
    }
    [[nodiscard]] quint16 getLocalPort() const override {
        qDebug() << "=== MOCK getLocalPort() called → returning" << mockLocalPort;
        return mockLocalPort;
    }
    [[nodiscard]] QHostAddress getPeerAddress() const override
    {
        return mockPeerAddress;
    }

    int disconnectFromHostCallCount = 0;
    void disconnectFromHost() override
    {
        disconnectFromHostCallCount++;
        qDebug() << "MockSslSocket::disconnectFromHost() called";
        qDebug() << "closeCallCount: " << disconnectFromHostCallCount;
        QSslSocket::disconnectFromHost();   // call base implementation
        qDebug() << "closeCallCount AFTER disconnectFromHost: " << disconnectFromHostCallCount;
    }

    int closeCallCount = 0;
    void close() override
    {
        closeCallCount++;
        qDebug() << "MockSslSocket::close() called";
        qDebug() << "closeCallCount: " << closeCallCount;
        QSslSocket::close();                // call base implementation
    }

    bool shouldCallConnectToHost = true;
    int connectToHostCallCount = 0;
    void connectToHost(const QString &hostName, quint16 port, OpenMode openMode = ReadWrite, NetworkLayerProtocol protocol = AnyIPProtocol) override
    {
        connectToHostCallCount++;

        lastConnectedToHostName = hostName;
        lastConnectedToPort = port;
        lastProtocol = protocol;

        if (shouldCallConnectToHost)
        {
            QSslSocket::connectToHost(hostName, port, openMode, protocol);
        }
    }

    QString lastConnectedToHostName;
    quint16 lastConnectedToPort = 0;
    NetworkLayerProtocol lastProtocol = QAbstractSocket::AnyIPProtocol;

private:
    bool mockConnected = false;
    bool mockEncrypted = false;
    bool isMockValid = false;
    QList<QSslError> mockSslErrors;
    QSslCipher mockCipher;
    QAbstractSocket::SocketState mockState = QAbstractSocket::UnconnectedState;
    qint64 mockBytesAvailable = 0;
    QHostAddress mockPeerAddress = QHostAddress("127.0.0.1");
    QByteArray mockReadData;
    quint16 mockPeerPort = 0;
    quint16 mockLocalPort = 0;
};
#endif //MOCKSSLSOCKET_H
