//
// Created by Tomas Gallucci on 6/18/26.
//

#ifndef INCOMINGTCPCONNECTIONTESTDOUBLE_H
#define INCOMINGTCPCONNECTIONTESTDOUBLE_H

#include "connections/incomingtcpconnection.h"

class IncomingTcpConnectionTestDouble : public IncomingTcpConnection
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
};

#endif //INCOMINGTCPCONNECTIONTESTDOUBLE_H

