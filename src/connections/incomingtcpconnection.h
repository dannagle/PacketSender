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

    void receiveData(const int socketDescriptor, const bool isSecure, const bool persistent) override;
    virtual std::unique_ptr<IncomingTcpThread> makeIncomingTcpThread(int socketDescriptor, bool isSecure, bool isPersistent);
private:
    signals:
        void sendRequested(Packet packet);
};


#endif //INCOMINGTCPCONNECTION_H
