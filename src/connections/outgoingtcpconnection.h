//
// Created by Tomas Gallucci on 6/18/26.
//

#ifndef OUTGOINGTCPCONNECTION_H
#define OUTGOINGTCPCONNECTION_H
#include "basetcpconnection.h"
#include "../outgoingtcpthread.h"


class OutgoingTcpConnection : public BaseTcpConnection
{
    Q_OBJECT

public:

    explicit OutgoingTcpConnection(QObject* parent = nullptr);

    void send(const Packet& packet) override;

protected:
    virtual std::unique_ptr<OutgoingTcpThread> makeOutgoingTcpThread(const Packet& packet);
};




#endif //OUTGOINGTCPCONNECTION_H
