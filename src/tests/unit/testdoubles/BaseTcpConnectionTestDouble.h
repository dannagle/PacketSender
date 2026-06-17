//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef CONNECTIONTESTDOUBLE_H
#define CONNECTIONTESTDOUBLE_H

#include <QObject>

#include "basetcpthreadtestdouble.h"
#include "../utils/calltracker.h"
#include "connections/basetcpconnection.h"
#include "tests/unit/utils/testutils.h"

class BaseTcpConnectionTestDouble : public BaseTcpConnection, public CallTracker
{
    Q_OBJECT

    explicit BaseTcpConnectionTestDouble(std::unique_ptr<BaseTcpThread> thread,
                                  QObject* parent = nullptr)
                : BaseTcpConnection(std::move(thread), parent)
    {

    }

    /** Convenience constructor that creates a mock thread internally */
    explicit BaseTcpConnectionTestDouble(QObject* parent = nullptr)
        : BaseTcpConnection(std::make_unique<BaseTcpThreadTestDouble>(
            TestUtils::createMockSocketForTest()), parent)
    {

    }

    ~BaseTcpConnectionTestDouble() override = default;

    void setThread(std::unique_ptr<BaseTcpThread> thread)
    {
        thread_ = std::move(thread);
    }

};

#endif //CONNECTIONTESTDOUBLE_H
