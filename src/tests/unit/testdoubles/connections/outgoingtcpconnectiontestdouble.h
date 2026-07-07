//
// Created by Tomas Gallucci on 6/20/26.
//

#ifndef OUTGOINGTCPCONNECTIONTESTDOUBLE_H
#define OUTGOINGTCPCONNECTIONTESTDOUBLE_H

#include <type_traits>

#include "../../connections/outgoingtcpconnection.h"
#include "../../outgoingtcpthread.h"
#include "../unit/testdoubles/outgoingtcpthreadtestdouble.h"

class OutgoingTcpConnectionTestDouble : public OutgoingTcpConnection, public CallTracker
{
    Q_OBJECT

public:
    explicit OutgoingTcpConnectionTestDouble(QObject *parent = nullptr)
        : OutgoingTcpConnection(parent)
    {

    }

    void setThread(std::unique_ptr<OutgoingTcpThread> thread)
    {
        thread_ = std::move(thread);
    }

    [[nodiscard]] OutgoingTcpThreadTestDouble& getThread() const
    {
        if (!thread_) {
            throw std::runtime_error("thread_ is null - send() was never called or failed to create the thread");
        }

        auto* t = dynamic_cast<OutgoingTcpThreadTestDouble*>(thread_.get());
        if (!t) {
            std::string className = thread_->metaObject()->className();
            throw std::runtime_error("thread_ is not an OutgoingTcpThreadTestDouble. It is " + className);
        }
        return *t;
    }

    enum class SocketConfigurationForTest
    {
        DEFAULT,
        HANDLE_OUTGOING_PLAIN_TCP_SUCCESS,
        HANDLE_OUTGOING_PLAIN_TCP_UNSUCCESSFUL,
        HANDLE_OUTGOING_SSL_SUCCESS,
        HANDLE_OUTGOING_SSL_UNSUCCESSFUL,
    };

    SocketConfigurationForTest desiredSocketConfigurationForTest = SocketConfigurationForTest::DEFAULT;

    std::vector<Connection::State> states;

    QString printStates() const
    {
        QStringList list;

        for (const auto state : states)
        {
            list << Connection::stateToString(state);
        }

        return list.join(",");
    }

protected:
    std::unique_ptr<OutgoingTcpThread> makeOutgoingTcpThread(const Packet& packet) override
    {
        qDebug() << "makeOutgoingTcpThread override in OutgoingTcpConnectionTestDouble";

        auto thread = std::make_unique<OutgoingTcpThreadTestDouble>(packet, this);
        thread->setSocketForTest(configureSocketForTest().release());

        qDebug() << "Test double created thread at address:" << thread.get();
        return thread;
    }

    void setupSignalConnections() override
    {
        recordCall(SETUP_SIGNAL_CONNECTIONS());
        OutgoingTcpConnection::setupSignalConnections();
    }

    void setState(State state) override
    {
        states.push_back(state);
        BaseTcpConnection::setState(state);
    }


private:
    std::unique_ptr<MockSslSocket> defaultSocketConditions()
    {
        auto mockSocket = std::make_unique<MockSslSocket>();

        mockSocket->setMockConnected(true);
        mockSocket->setIsValid(true);
        mockSocket->setMockState(QAbstractSocket::ConnectedState);

        return mockSocket;
    }

    std::unique_ptr<MockSslSocket> makeHandlePlainTcpSuccessful()
    {
        QDEBUG() << "called makeHandlePlainTcpSuccessful()";
        auto mockSocket = defaultSocketConditions();

        mockSocket->shouldCallConnectToHost = false;

        return mockSocket;
    }

    std::unique_ptr<MockSslSocket> makeHandlePlainTcpUnsuccessful()
    {
        auto mockSocket = makeHandlePlainTcpSuccessful();
        mockSocket->makeWaitForConnectedReturnFalse = true;

        return mockSocket;
    }

    std::unique_ptr<MockSslSocket> makeHandleSslTcpSuccessful()
    {
        QDEBUG() << "called makeHandlePlainTcpSuccessful()";
        auto mockSocket = defaultSocketConditions();

        mockSocket->setMockEncrypted(true);
        mockSocket->shouldCallConnectToHost = false;

        return mockSocket;
    }

    std::unique_ptr<MockSslSocket> makeHandleSslTcpUnsuccessful()
    {
        QDEBUG() << "called makeHandlePlainTcpSuccessful()";
        auto mockSocket = defaultSocketConditions();

        mockSocket->setMockEncrypted(true);

        QList<QSslError> errors = { QSslError(QSslError::SelfSignedCertificate) };
        mockSocket->setMockSslErrors(errors);

        mockSocket->shouldCallConnectToHost = false;
        mockSocket->makeWaitForConnectedReturnFalse = true;

        return mockSocket;
    }

    std::unique_ptr<MockSslSocket> configureSocketForTest()
    {
        switch (desiredSocketConfigurationForTest)
        {
            case SocketConfigurationForTest::DEFAULT: return defaultSocketConditions();
            case SocketConfigurationForTest::HANDLE_OUTGOING_PLAIN_TCP_SUCCESS: return makeHandlePlainTcpSuccessful();
            case SocketConfigurationForTest::HANDLE_OUTGOING_PLAIN_TCP_UNSUCCESSFUL: return makeHandlePlainTcpUnsuccessful();
            case SocketConfigurationForTest::HANDLE_OUTGOING_SSL_SUCCESS: return makeHandleSslTcpSuccessful();
            case SocketConfigurationForTest::HANDLE_OUTGOING_SSL_UNSUCCESSFUL: return makeHandleSslTcpUnsuccessful();
        }
    }
};

#endif //OUTGOINGTCPCONNECTIONTESTDOUBLE_H
