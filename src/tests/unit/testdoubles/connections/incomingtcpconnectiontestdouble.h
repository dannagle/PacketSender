//
// Created by Tomas Gallucci on 6/18/26.
//

#ifndef INCOMINGTCPCONNECTIONTESTDOUBLE_H
#define INCOMINGTCPCONNECTIONTESTDOUBLE_H

#include "connections/incomingtcpconnection.h"
#include "testdoubles/incomingtcpthreadtestdouble.h"

class IncomingTcpConnectionTestDouble : public IncomingTcpConnection, public CallTracker
{
    Q_OBJECT
public:
    explicit IncomingTcpConnectionTestDouble(QObject* parent=nullptr)
        : IncomingTcpConnection(parent)
    {

    }

    void setThread(std::unique_ptr<IncomingTcpThread> thread)
    {
        thread_ = std::move(thread);
    }
    
    [[nodiscard]] IncomingTcpThreadTestDouble& getThread() const
    {
        if (!thread_) {
            throw std::runtime_error("thread_ is null - receiveData() was never called or failed to create the thread");
        }

        auto* t = dynamic_cast<IncomingTcpThreadTestDouble*>(thread_.get());
        if (!t) {
            std::string className = thread_->metaObject()->className();
            throw std::runtime_error("thread_ is not an IncomingTcpThreadTestDouble. It is " + className);
        }
        return *t;
    }

    enum class SocketConfigurationForTest
    {
        HANDLE_INCOMING_PLAIN_TCP_SUCCESS,
        HANDLE_INCOMING_PLAIN_TCP_UNSUCCESSFUL_READ,
        HANDLE_INCOMING_SSL_SUCCESS,
        HANDLE_INCOMING_SSL_FAILURE
    };

    SocketConfigurationForTest desiredSocketConfigurationForTest = SocketConfigurationForTest::HANDLE_INCOMING_PLAIN_TCP_SUCCESS;

    std::unique_ptr<IncomingTcpThread> makeIncomingTcpThread(int socketDescriptor, bool isSecure, bool isPersistent) override
    {
        auto newThread = std::make_unique<IncomingTcpThreadTestDouble>(socketDescriptor, this);
        newThread->setPersistent(isPersistent);
        newThread->setShouldUseSSL(isSecure);

        // Configure the mock socket before passing it to the thread
        auto mockSocket = configureSocketForTest();
        newThread->setSocketForTest(mockSocket.release());

        return newThread;
    }

protected:
    void setupSignalConnections() override
    {
        recordCall(SETUP_SIGNAL_CONNECTIONS());
        IncomingTcpConnection::setupSignalConnections();
    }

private:
    std::unique_ptr<MockSslSocket> configureSocketForTest()
    {
        switch (desiredSocketConfigurationForTest)
        {
        case SocketConfigurationForTest::HANDLE_INCOMING_PLAIN_TCP_SUCCESS:
            return createIncomingPlainTcpSuccessful();
        case SocketConfigurationForTest::HANDLE_INCOMING_PLAIN_TCP_UNSUCCESSFUL_READ:
            return createIncomingPlainTcpSocketUnconnected();
        case SocketConfigurationForTest::HANDLE_INCOMING_SSL_SUCCESS:
            return createSSlSuccessful();
        case SocketConfigurationForTest::HANDLE_INCOMING_SSL_FAILURE:
            return createSslFailure();
        }

        // Fallback
        return std::make_unique<MockSslSocket>();
    }

    std::unique_ptr<MockSslSocket> createIncomingPlainTcpSuccessful()
    {
        auto mockSock = std::make_unique<MockSslSocket>();

        mockSock->setIsValid(true);
        mockSock->setMockState(QAbstractSocket::ConnectedState);
        mockSock->setMockConnected(true);

        return mockSock;
    }

    std::unique_ptr<MockSslSocket> createSSlSuccessful()
    {
        auto mockSock = createIncomingPlainTcpSuccessful();
        mockSock->setMockEncrypted(true);
        return mockSock;
    }

    std::unique_ptr<MockSslSocket> createSslFailure()
    {
        auto mockSock = createIncomingPlainTcpSuccessful();
        mockSock->setMockEncrypted(true);

        const QList<QSslError> errors = { QSslError(QSslError::SelfSignedCertificate) };
        mockSock->setMockSslErrors(errors);

        return mockSock;
    }

    std::unique_ptr<MockSslSocket> createIncomingPlainTcpSocketUnconnected()
    {
        auto mockSock = std::make_unique<MockSslSocket>();

        mockSock->setIsValid(true);
        mockSock->setMockState(QAbstractSocket::UnconnectedState);
        mockSock->setMockConnected(false);

        return mockSock;
    }

};

#endif //INCOMINGTCPCONNECTIONTESTDOUBLE_H

