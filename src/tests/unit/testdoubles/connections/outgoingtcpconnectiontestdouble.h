//
// Created by Tomas Gallucci on 6/20/26.
//

#ifndef OUTGOINGTCPCONNECTIONTESTDOUBLE_H
#define OUTGOINGTCPCONNECTIONTESTDOUBLE_H

#include <type_traits>

#include "../../connections/outgoingtcpconnection.h"
#include "../../outgoingtcpthread.h"
#include "../unit/testdoubles/outgoingtchpthreadtestdouble.h"

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

protected:
    std::unique_ptr<OutgoingTcpThread> makeOutgoingTcpThread(const Packet& packet) override
    {
        qDebug() << "makeOutgoingTcpThread override in OutgoingTcpConnectionTestDouble";
        return std::make_unique<OutgoingTcpThreadTestDouble>(packet, this);
    }

    void setupSignalConnections() override
    {
        recordCall(SETUP_SIGNAL_CONNECTIONS());
        OutgoingTcpConnection::setupSignalConnections();
    }
};

static_assert(!std::is_abstract_v<OutgoingTcpConnectionTestDouble>,
          "This class is still abstract");

#endif //OUTGOINGTCPCONNECTIONTESTDOUBLE_H
