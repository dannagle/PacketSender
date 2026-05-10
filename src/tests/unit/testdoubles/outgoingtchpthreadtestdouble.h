//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCHPTHREADTESTDOUBLE_H
#define OUTGOINGTCHPTHREADTESTDOUBLE_H
#include "outgoingtcpthread.h"

class OutgoingTcpThreadTestDouble : public OutgoingTcpThread
{
public:
    explicit OutgoingTcpThreadTestDouble(
        QSslSocket* socket, const Packet& packetToSend): OutgoingTcpThread(socket, packetToSend)
    {}

    // Convenience constructor
    explicit OutgoingTcpThreadTestDouble(const Packet& packetToSend)
        : OutgoingTcpThreadTestDouble(new QSslSocket(), packetToSend)   // delegates to main constructor
    {}

    Packet& getSendPacketByReference()
    {
        return this->sendPacket;
    }

    bool isSocketValid() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(socket))
        {
            return mock->isValid();
        }

        return BaseTcpThread::isSocketValid();
    }
};

#endif //OUTGOINGTCHPTHREADTESTDOUBLE_H
