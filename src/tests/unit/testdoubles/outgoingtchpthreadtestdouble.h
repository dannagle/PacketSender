//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCHPTHREADTESTDOUBLE_H
#define OUTGOINGTCHPTHREADTESTDOUBLE_H
#include "../../outgoingtcpthread.h"

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
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket()))
        {
            return mock->isValid();
        }

        return BaseTcpThread::isSocketValid();
    }

    QAbstractSocket::SocketState getSocketState() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket()))
        {
            return mock->getMockState();
        }

        return BaseTcpThread::getSocketState();
    }

    int prepareOutgoingPacketCallCount = 0;
    int sendOutgoingPacketCallCount = 0;
    int closeConnectionCallCount = 0;
    int sleepCallCount = 0;

    const std::vector<QString>& getCallSequence() const { return callSequence; }
    void resetCallTracking()
    {
        prepareOutgoingPacketCallCount
            = sendOutgoingPacketCallCount
            = closeConnectionCallCount
            = sleepCallCount
            = 0;
        callSequence.clear();
    }

    void callPrepareOutgoingSendPacket()
    {
        prepareOutgoingPacket();
    }

    void callCloseConnection()
    {
        closeConnection();
    }

    void callRun()
    {
        OutgoingTcpThread::run();
    }

protected:

    void prepareOutgoingPacket() override
    {
        prepareOutgoingPacketCallCount++;
        callSequence.push_back("prepareOutgoingPacket");
        OutgoingTcpThread::prepareOutgoingPacket();
    }

    void sendOutgoingPacket() override
    {
        sendOutgoingPacketCallCount++;
        callSequence.push_back("sendOutgoingPacket");
        OutgoingTcpThread::sendOutgoingPacket();
    }

    void closeConnection() override
    {
        closeConnectionCallCount++;
        callSequence.push_back("closeConnection");
        OutgoingTcpThread::closeConnection();
    }

    void sleep(unsigned long usecs) override
    {
        sleepCallCount++;
        callSequence.push_back("usleep " + QString::number(usecs) + " usecs");
        OutgoingTcpThread::sleep(usecs);
    }

private:
    std::vector<QString> callSequence;
};

#endif //OUTGOINGTCHPTHREADTESTDOUBLE_H
