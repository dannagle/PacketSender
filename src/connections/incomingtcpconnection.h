//
// Created by Tomas Gallucci on 6/17/26.
//

#ifndef INCOMINGTCPCONNECTION_H
#define INCOMINGTCPCONNECTION_H
#include "basetcpconnection.h"
#include "incomingtcpthread.h"


class IncomingTcpConnection : public BaseTcpConnection
{
    Q_OBJECT
public:
    explicit IncomingTcpConnection(QObject* parent = nullptr);

};


#endif //INCOMINGTCPCONNECTION_H
