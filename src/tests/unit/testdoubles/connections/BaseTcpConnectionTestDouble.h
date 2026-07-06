//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef BASETCPCONNECTIONTESTDOUBLE_H
#define BASETCPCONNECTIONTESTDOUBLE_H

#include <QObject>

#include "../basetcpthreadtestdouble.h"
#include "../../utils/calltracker.h"
#include "connections/basetcpconnection.h"
#include "tests/unit/utils/testutils.h"

class BaseTcpConnectionTestDouble : public BaseTcpConnection, public CallTracker
{
    Q_OBJECT
public:

    explicit BaseTcpConnectionTestDouble(QObject* parent = nullptr)
                : BaseTcpConnection(parent)
    {

    }

    /** Convenience constructor that creates a mock thread internally */
    // explicit BaseTcpConnectionTestDouble(QObject* parent = nullptr)
    //     : BaseTcpConnection(std::make_unique<BaseTcpThreadTestDouble>(
    //         TestUtils::createMockSocketForTest()), parent)
    // {
    //
    // }

    ~BaseTcpConnectionTestDouble() override = default;

    void setThread(std::unique_ptr<BaseTcpThread> thread)
    {
        thread_ = std::move(thread);
    }

    BaseTcpThread& getThreadByReference() const
    {
        return *thread_;
    }

    State getState() const
    {
        return state_;
    }

    // ═════════════════════════════════════════════════════════════════════════════
    //                              CALL* SECTION
    // ═════════════════════════════════════════════════════════════════════════════
    //                              All call* methods
    // ═════════════════════════════════════════════════════════════════════════════

    [[nodiscard]] QString callGetClassName() const
    {
        return BaseTcpConnection::getClassName();
    }

    void callTerminateConnection()
    {
        terminateConnection();
    }

protected:
    /****************************************************************************************
     *                                                                                      *
     *                                  OVERRIDES SECTION                                   *
     *                                                                                      *
     *  All overridden virtual methods follow this exact pattern:                           *
     *                                                                                      *
     *     1. Record the method call                                                        *
     *     2. Forward the call to the real implementation (if set)                          *
     *     3. Return the result                                                             *
     *                                                                                      *
     ****************************************************************************************/
    void terminateConnection() override
    {
        recordCall(TERMINATE_CONNECTION());
        BaseTcpConnection::terminateConnection();
    }

};

#endif //BASETCPCONNECTIONTESTDOUBLE_H
