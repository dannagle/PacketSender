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

    // Hide base member with derived type
    MockSslSocket *clientConnection;

    // Expose the protected getters as public for easy test use
    using TCPThread::getClientConnection;
    using TCPThread::getSocketDescriptor;
    using TCPThread::getIsSecure;
    using TCPThread::getIncomingPersistent;
    using TCPThread::getHost;
    using TCPThread::getPort;
    using TCPThread::getSendFlag;
    using TCPThread::getManagedByConnection;
    using TCPThread::getIPConnectionProtocol;
    using TCPThread::clientSocket;

    // Optional: add test-specific methods if needed, e.g.
    // bool isThreadStarted() const { return isRunning(); }  // example

    void set_m_managedByConnection(bool isManagedByConnection) {this->m_managedByConnection = isManagedByConnection;};

    void setSendPacketToIp(QString toIp) {sendPacket.toIP = toIp;};
    void setClientConnection(QSslSocket *sock)
    {
        clientConnection = dynamic_cast<MockSslSocket*>(sock);
        if (!clientConnection && sock) {
            qWarning() << "setClientConnection: sock is not a MockSslSocket instance";
        }
    }

    bool fireTryConnectEncrypted() { return tryConnectEncrypted(); }

    // for spying / verification
    int outgoingSSLCallCount = 0;
    int incomingSSLCallCount = 0;

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
};



#endif //TESTTCPTHREADCLASS_H
