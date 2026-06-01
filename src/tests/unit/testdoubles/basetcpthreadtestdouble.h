//
// Created by Tomas Gallucci on 4/26/26.
//

#ifndef BASETCPTHREADTESTDOUBLE_H
#define BASETCPTHREADTESTDOUBLE_H

#include "MockSslSocket.h"
#include "../../../basetcpthread.h"

class BaseTcpThreadTestDouble : public BaseTcpThread
{
    Q_OBJECT

public:
    explicit BaseTcpThreadTestDouble(PacketSenderQSslSocketInterface *socketInterface, QObject* parent = nullptr)
        : BaseTcpThread(socketInterface, parent)
    {
    }

    void setSocketForTest(PacketSenderQSslSocketInterface* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    void callSendOutgoingPacket(Packet &packet)
    {
        sendOutgoingPacket(packet);
    }

protected:
    // Minimal implementation of the pure virtual method
    void run() override
    {
        // Do nothing for most tests, or QThread::run() if you want default behavior
    }

    // Minimal implementation of the pure virtual method
    void closeConnection() override
    {
        // Do nothing
    }
};

#endif //BASETCPTHREADTESTDOUBLE_H
