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

    // Minimal implementation of the pure virtual method
    void run() override
    {
        // Do nothing for most tests, or QThread::run() if you want default behavior
    }

    void setSocketForTest(QSslSocket* newSocket)
    {
        socket = newSocket;
    }

    bool isSocketEncrypted() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(socket)) {
            return mock->isEncrypted();
        }
        return BaseTcpThread::isSocketEncrypted();
    }

    quint16 getPeerPort() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(socket)) {
            return mock->getPeerPort();
        }

        return BaseTcpThread::getPeerPort();
    }

    // [[nodiscard]] QAbstractSocket::NetworkLayerProtocol getIPConnectionProtocol() const override
    // {
    //     if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(socket)) {
    //         return mock->getIPConnectionProtocol();
    //     }
    //
    //     return BaseTcpThread::getIPConnectionProtocol();
    // }
    //
    // [[nodiscard]] QString getPeerAddressAsString() const override
    // {
    //     if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(socket)) {
    //         return mock->getIPConnectionProtocol();
    //     }
    //
    //     return BaseTcpThread::getIPConnectionProtocol();
    // }

protected:
    QHostAddress getSocketPeerAddress() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(socket)) {
            return mock->getMockPeerAddress();
        }
        return BaseTcpThread::getSocketPeerAddress();
    }
};

#endif //BASETCPTHREADTESTDOUBLE_H
