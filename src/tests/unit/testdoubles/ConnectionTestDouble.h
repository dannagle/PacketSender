//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef CONNECTIONTESTDOUBLE_H
#define CONNECTIONTESTDOUBLE_H

#include <QObject>

#include "basetcpthreadtestdouble.h"
#include "../../src/connection.h"
#include "../../src/basetcpthread.h"
#include "../utils/calltracker.h"
#include "tests/unit/utils/testutils.h"

class ConnectionTestDouble : public Connection, public CallTracker
{
    Q_OBJECT

    explicit ConnectionTestDouble(std::unique_ptr<BaseTcpThread> thread,
                                  QObject* parent = nullptr)
                : Connection(std::move(thread), parent)
    {

    }

    /** Convenience constructor that creates a mock thread internally */
    explicit ConnectionTestDouble(QObject* parent = nullptr)
        : Connection(std::make_unique<BaseTcpThreadTestDouble>(
            TestUtils::createMockSocketForTest()), parent)
    {

    }

    ~ConnectionTestDouble() override = default;

    void setThread(std::unique_ptr<BaseTcpThread> thread)
    {
        thread_ = std::move(thread);
    }

};

#endif //CONNECTIONTESTDOUBLE_H
