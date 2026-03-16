//
// Created by Tomas Gallucci on 3/6/26.
//

#ifndef TESTTCPTHREADCLASS_H
#define TESTTCPTHREADCLASS_H

#include "tcpthread.h"

class TestTcpThreadClass : public TCPThread
{
public:
    explicit TestTcpThreadClass(int socketDescriptor,
                           bool isSecure,
                           bool isPersistent,
                           QObject *parent = nullptr)
        : TCPThread(socketDescriptor, isSecure, isPersistent, parent)
    {

        destructorWaitMs = 500;
    }

    explicit TestTcpThreadClass(const QString &host,
                                quint16 port,
                                const Packet &initialPacket = Packet(),
                                QObject *parent = nullptr)
        : TCPThread(host, port, initialPacket, parent)
    {
        destructorWaitMs = 500;
    }

    void forceFastExitFromPersistentLoop()
    {
        closeRequest = true;
        qDebug() << "MOCK: Forced immediate exit via closeRequest";
    }
    // Expose the protected getters as public for easy test use
    using TCPThread::getClientConnection;
    using TCPThread::getSocketDescriptor;
    using TCPThread::getIsSecure;
    using TCPThread::getIncomingPersistent;
    using TCPThread::getHost;
    using TCPThread::getPort;
    using TCPThread::getSendFlag;
    using TCPThread::getManagedByConnection;

    // Optional: add test-specific methods if needed, e.g.
    // bool isThreadStarted() const { return isRunning(); }  // example

    void set_m_managedByConnection(bool isManagedByConnection) {this->m_managedByConnection = isManagedByConnection;};

protected:
    [[nodiscard]] bool divideWaitBy10ForUnitTest() const override { return true; }
};


#endif //TESTTCPTHREADCLASS_H
