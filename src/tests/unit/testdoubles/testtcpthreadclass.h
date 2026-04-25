//
// Created by Tomas Gallucci on 3/6/26.
//

#ifndef TESTTCPTHREADCLASS_H
#define TESTTCPTHREADCLASS_H

#include "tcpthread.h"

#include <QString>

#include "MockSslSocket.h"

class TestTcpThreadClass : public TCPThread
{
public:
    explicit TestTcpThreadClass(int socketDescriptor,
                           bool isSecure,
                           bool isPersistent,
                           QObject *parent = nullptr)
        : TCPThread(socketDescriptor, isSecure, isPersistent, parent)
    {

        destructorWaitMs = 500;
    }

    explicit TestTcpThreadClass(const QString &host,
                                quint16 port,
                                const Packet &initialPacket = Packet(),
                                QObject *parent = nullptr)
        : TCPThread(host, port, initialPacket, parent)
    {
        destructorWaitMs = 500;
    }

    explicit TestTcpThreadClass(QSslSocket *preCreatedSocket,
                            const QString &host,
                            quint16 port,
                            const Packet &initialPacket = Packet(),
                            QObject *parent = nullptr)
        : TCPThread(preCreatedSocket, host, port, initialPacket, parent)
    {
        destructorWaitMs = 500;
    }

    void forceFastExitFromPersistentLoop()
    {
        closeRequest = true;
        qDebug() << "MOCK: Forced immediate exit via closeRequest";
    }

    // Expose the protected getters as public for easy test use
    using TCPThread::getClientConnection;
    using TCPThread::getSocketDescriptor;
    using TCPThread::getIsSecure;
    using TCPThread::getIncomingPersistent;
    using TCPThread::getHost;
    using TCPThread::getPort;
    using TCPThread::getSendFlag;
    using TCPThread::getManagedByConnection;
    using TCPThread::clientSocket;
    using TCPThread::setSocketDescriptor;
    using TCPThread::insidePersistent;

    // Optional: add test-specific methods if needed, e.g.
    // bool isThreadStarted() const { return isRunning(); }  // example

    MockSslSocket* getMockSocket()
    {
        MockSslSocket* mock = dynamic_cast<MockSslSocket*>(getClientConnection());
        if (!mock && getClientConnection() != nullptr) {
            qWarning() << "getMockSocket: clientConnection is not a MockSslSocket!";
        }
        return mock;
    }

    void set_m_managedByConnection(bool isManagedByConnection) {this->m_managedByConnection = isManagedByConnection;};

    void setSendPacketToIp(QString toIp) {sendPacket.toIP = toIp;};
    void setClientConnection(QSslSocket *sock)
    {
        // Update the base class member (this is what the real code uses)
        TCPThread::clientConnection = sock;
    }

    void setMockIPProtocol(QAbstractSocket::NetworkLayerProtocol protocol)
    {
        mockIPProtocol = protocol;
        mockIPProtocolSet = true;
    }

    bool fireTryConnectEncrypted() { return tryConnectEncrypted(); }

    // for spying / verification
    int outgoingSSLCallCount = 0;
    int incomingSSLCallCount = 0;
    int buildInitialReceivedPacketCallCount = 0;
    int prepareForPersistentLoopCallCount = 0;
    int persistentConnectionLoopCallCount = 0;
    int cleanupAfterPersistentConnectionLoopCallCount = 0;
    int deleteLaterCallCount = 0;
    int sendCurrentPacketCallCount = 0;
    int handleReceiveBeforeSendCallCount = 0;
    int buildReceivedPacketCallCount = 0;
    int handleResponseAfterSendCallCount = 0;
    int shouldBreakPersistentLoopCallCount = 0;
    int resetPacketForPersistentLoopCallCount = 0;

    bool forceExitAfterOneIteration = false;

    // Test helpers to call protected SSL handlers
    void callHandleOutgoingSSLHandshake(bool handshakeSucceeded, bool isEncryptedResult)
    {
        outgoingSSLCallCount++;
        handleOutgoingSSLHandshake(handshakeSucceeded, isEncryptedResult);
    }

    void callHandleIncomingSSLHandshake(QSslSocket &sock)
    {
        incomingSSLCallCount++;
        handleIncomingSSLHandshake(sock);
    }

    void callRunOutgoingClient()
    {
        TCPThread::runOutgoingClient(); // for clarity until we override
    }

    Packet callBuildInitialReceivedPacket(QSslSocket &sock)
    {
        buildInitialReceivedPacketCallCount++;
        return buildInitialReceivedPacket(sock);
    }

    void callPrepareForPersistentLoop(const Packet &initialPacket)
    {
        prepareForPersistentLoopCallCount++;
        prepareForPersistentLoop(initialPacket);
    };

    void callPersistentConnectionLoop()
    {
        persistentConnectionLoopCallCount++;
        persistentConnectionLoop();
    }

    void callCleanupAfterPersistentConnectionLoop()
    {
        cleanupAfterPersistentConnectionLoopCallCount++;
        cleanupAfterPersistentConnectionLoop();
    }

    void callSendCurrentPacket()
    {
        sendCurrentPacketCallCount++;
        sendCurrentPacket();
    }

    void callHandleReceiveBeforeSend()
    {
        handleReceiveBeforeSendCallCount++;
        handleReceiveBeforeSend();
    }

    Packet callBuildReceivedPacket()
    {
        buildReceivedPacketCallCount++;
        return buildReceivedPacket();
    }

    void callHandleResponseAfterSend(Packet &receivedPacket)
    {
        handleResponseAfterSendCallCount++;
        handleResponseAfterSend(receivedPacket);
    }

    bool callShouldBreakPersistentLoop()
    {
        shouldBreakPersistentLoopCallCount++;
        return shouldBreakPersistentLoop();
    }

    void callResetPacketForPersistentLoop()
    {
        resetPacketForPersistentLoopCallCount++;
        resetPacketForPersistentLoop();
    }

    Packet getSendPacket() { return sendPacket; };
    Packet& getSendPacketByReference() { return sendPacket; };

    QAbstractSocket::NetworkLayerProtocol getIPConnectionProtocol() const override
    {
        if (mockIPProtocolSet) {
            return mockIPProtocol;
        }
        return TCPThread::getIPConnectionProtocol();
    }

protected:
    [[nodiscard]] bool divideWaitBy10ForUnitTest() const override { return true; }

    bool checkConnectionAndEncryption() override
    {
        MockSslSocket *mock = qobject_cast<MockSslSocket*>(clientSocket());
        if (!mock) {
            qWarning() << "No mock in test — falling back to base";
            return TCPThread::checkConnectionAndEncryption();
        }

        bool connected = mock->waitForConnected(5000);
        bool encrypted = mock->waitForEncrypted(5000);
        bool isEncrypted = mock->isEncrypted();

        qDebug() << "from checkConnectionAndEncryption Test mock: connected =" << connected;
        qDebug() << "from checkConnectionAndEncryption Test mock: encrypted =" << encrypted;
        qDebug() << "from checkConnectionAndEncryption Test mock: isEncrypted =" << isEncrypted;

        return connected && encrypted;
    }

    std::pair<bool, bool> performEncryptedHandshake() override
    {
        MockSslSocket *mock = qobject_cast<MockSslSocket*>(clientSocket());
        if (!mock) return TCPThread::performEncryptedHandshake();

        bool connected = mock->waitForConnected(5000);
        bool encrypted = mock->waitForEncrypted(5000);
        bool isEnc = mock->isEncrypted();

        qDebug() << "Test mock handshake: connected =" << connected
                 << "encrypted =" << encrypted
                 << "isEncrypted =" << isEnc;

        return {connected, encrypted};
    }

    bool isSocketEncrypted(const QSslSocket &sock) const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(&sock)) {
            qDebug() << "TestTcpThreadClass::isSocketEncrypted - using mock value:" << mock->isEncrypted();
            return mock->isEncrypted();
        }

        return TCPThread::isSocketEncrypted(sock);
    }

    QList<QSslError> getSslErrors(QSslSocket* sock) const override
    {
        if (MockSslSocket *mock = qobject_cast<MockSslSocket*>(sock)) {
            return mock->sslErrors();
        }
        return TCPThread::getSslErrors(sock);
    }

    QList<QSslError> getSslHandshakeErrors(QSslSocket* sock) const override
    {
        if (MockSslSocket *mock = qobject_cast<MockSslSocket*>(sock)) {
            return mock->sslErrors();
        }
        return TCPThread::getSslHandshakeErrors(sock);
    }

    bool shouldContinuePersistentLoop() const override
    {
        QDEBUG() << "closeRequest from shouldContinuePersistentLoop" << closeRequest;

        if (forceExitAfterOneIteration) {
            if (persistentLoopIterationCount == 0) {
                qDebug() << "Test double: allowing first full iteration";
                persistentLoopIterationCount++;
                return true;                    // allow the loop body to run once
            } else {
                qDebug() << "Test double: forcing exit after one iteration";
                return false;
            }
        }

        return TCPThread::shouldContinuePersistentLoop();
    }

    QAbstractSocket::SocketState socketState() const override
    {
        // Prefer the mock if we have one injected
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(clientSocket())) {
            return mock->getMockState();   // we'll add this getter
        }

        // Fall back to real implementation
        return TCPThread::socketState();
    }

    qint64 socketBytesAvailable() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(clientSocket())) {
            return mock->getMockBytesAvailable();
        }
        return TCPThread::socketBytesAvailable();
    }

    void deleteSocketLater() override
    {
        deleteLaterCallCount++;
        TCPThread::deleteSocketLater();
    }

    QHostAddress getPeerAddress() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(clientSocket())) {
            return mock->getMockPeerAddress();
        }
        return TCPThread::getPeerAddress();
    }

    QByteArray readSocketData() override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(clientSocket())) {
            return mock->getMockReadData();
        }
        return TCPThread::readSocketData();
    }

    quint16 getPeerPort() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(clientSocket())) {
            return mock->getPeerPort();   // we'll add this to the mock
        }
        return TCPThread::getPeerPort();
    }

private:
    mutable short persistentLoopIterationCount = 0;
    bool mockIPProtocolSet = false;
    QAbstractSocket::NetworkLayerProtocol mockIPProtocol = QAbstractSocket::IPv4Protocol;
};



#endif //TESTTCPTHREADCLASS_H
