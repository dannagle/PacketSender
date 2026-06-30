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
            throw std::runtime_error("thread_ is null - send() was never called or failed to create the thread");
        }

        auto* t = dynamic_cast<IncomingTcpThreadTestDouble*>(thread_.get());
        if (!t) {
            std::string className = thread_->metaObject()->className();
            throw std::runtime_error("thread_ is not an IncomingTcpThreadTestDouble. It is " + className);
        }
        return *t;
    }

    std::unique_ptr<IncomingTcpThread> makeIncomingTcpThread(int socketDescriptor, bool isSecure, bool isPersistent) override
    {
        auto newThread = std::make_unique<IncomingTcpThreadTestDouble>(socketDescriptor, this);
        newThread->setPersistent(isPersistent);
        dynamic_cast<MockSslSocket*>(newThread->getSocketInterface())->setMockEncrypted(isSecure);
        return newThread;
    }

protected:
    void setupSignalConnections() override
    {
        recordCall(SETUP_SIGNAL_CONNECTIONS());
        IncomingTcpConnection::setupSignalConnections();
    }
};

#endif //INCOMINGTCPCONNECTIONTESTDOUBLE_H

