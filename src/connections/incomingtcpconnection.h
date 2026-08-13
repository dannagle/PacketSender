//
// Created by Tomas Gallucci on 6/17/26.
//

#ifndef INCOMINGTCPCONNECTION_H
#define INCOMINGTCPCONNECTION_H
#include "basetcpconnection.h"
#include "../tcpThreads/incomingtcpthread.h"


class IncomingTcpConnection : public BaseTcpConnection
{
    Q_OBJECT
public:
    explicit IncomingTcpConnection(QObject* parent = nullptr);

    void receiveData(int socketDescriptor, bool isSecure, bool persistent) override;
    virtual std::unique_ptr<IncomingTcpThread> makeIncomingTcpThread(int socketDescriptor, bool isSecure, bool isPersistent);

    void send(const Packet& packet) override;

private:
    signals:
        void sendRequested(Packet packet);
};


#endif //INCOMINGTCPCONNECTION_H
