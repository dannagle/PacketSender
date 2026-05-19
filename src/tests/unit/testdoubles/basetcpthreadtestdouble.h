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
    explicit BaseTcpThreadTestDouble(QSslSocket* socket, QObject* parent = nullptr)
        : BaseTcpThread(socket, parent)
    {
    }

    void setSocketForTest(QSslSocket* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    bool isSocketEncrypted() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket())) {
            return mock->isEncrypted();
        }
        return BaseTcpThread::isSocketEncrypted();
    }

    quint16 getPeerPort() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket())) {
            return mock->getPeerPort();
        }

        return BaseTcpThread::getPeerPort();
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

    QHostAddress getSocketPeerAddress() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket())) {
            return mock->getMockPeerAddress();
        }
        return BaseTcpThread::getSocketPeerAddress();
    }

    [[nodiscard]] QAbstractSocket::SocketState getSocketState() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket())) {
            return mock->getMockState();
        }

        return BaseTcpThread::getSocketState();
    }
};

#endif //BASETCPTHREADTESTDOUBLE_H
