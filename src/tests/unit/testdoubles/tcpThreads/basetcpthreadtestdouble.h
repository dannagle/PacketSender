//
// Created by Tomas Gallucci on 4/26/26.
//

#ifndef BASETCPTHREADTESTDOUBLE_H
#define BASETCPTHREADTESTDOUBLE_H

#include "../MockSslSocket.h"
#include "../../../../tcpThreads/basetcpthread.h"

class BaseTcpThreadTestDouble : public BaseTcpThread, public CallTracker
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

    void setSocketConnectionState(QAbstractSocket::SocketState mockSocketState)
    {
        auto& basePtr = getSocketPtrByReference();
        if (typeid(basePtr) == typeid(MockSslSocket))
        {
            auto* mockSocket = dynamic_cast<MockSslSocket*>(basePtr.get());
            mockSocket->setMockState(mockSocketState);
        }
    }

    bool callIsValidForSending(Packet& packet, QString* errorMessage = nullptr) const
    {
        return isValidForSending(packet, errorMessage);
    }

    void callSendOutgoingPacket(Packet &packet)
    {
        sendOutgoingPacket(packet);
    }

    void callCloseConnection()
    {
        closeConnection();
    }

protected:
    // Minimal implementation of the pure virtual method
    void run() override
    {
        // Do nothing for most tests, or QThread::run() if you want default behavior
    }

    bool isValidForSending(Packet& packet, QString* errorMessage) const override
    {
        recordCall(BASETCPTHREAD_ISVALIDFORSENDING());
        return BaseTcpThread::isValidForSending(packet, errorMessage);
    }

    void closeConnection() override
    {
        recordCall(CLOSE_CONNECTION());
        BaseTcpThread::closeConnection();
    }
};

#endif //BASETCPTHREADTESTDOUBLE_H
