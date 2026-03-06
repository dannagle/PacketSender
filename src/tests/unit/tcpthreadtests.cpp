//
// Created by Tomas Gallucci on 3/6/26.
//

#include "tcpthreadtests.h"

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
};
