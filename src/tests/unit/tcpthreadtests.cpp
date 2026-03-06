//
// Created by Tomas Gallucci on 3/6/26.
//

#include <QtTest/QTest.h>
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

void TcpThreadTests::testIncomingConstructorBasic()
{
    // Use invalid descriptor (real ones are positive; -1 is common sentinel)
    const TestTcpThreadClass thread(-1, /*isSecure*/ false, /*isPersistent*/ true);

    QVERIFY(thread.getClientConnection() != nullptr);
    QVERIFY(qobject_cast<QSslSocket*>(thread.getClientConnection()) != nullptr);

    QCOMPARE(thread.getSocketDescriptor(), -1);
    QCOMPARE(thread.isSecure, false);
    QCOMPARE(thread.incomingPersistent, true);
    QCOMPARE(thread.getHost(), QString());
    QCOMPARE(thread.getPort(), quint16(0));

    // Optional: check other defaults
    QCOMPARE(thread.sendFlag, false);
    QCOMPARE(thread.consoleMode, false);
    QCOMPARE(thread.getManagedByConnection(), true);
}

void TcpThreadTests::testIncomingConstructorWithSecureFlag()
{
    const TestTcpThreadClass thread(12345, /*isSecure*/ true, /*isPersistent*/ false);

    QVERIFY(thread.getClientConnection() != nullptr);
    QVERIFY(qobject_cast<QSslSocket*>(thread.getClientConnection()) != nullptr);

    QCOMPARE(thread.getSocketDescriptor(), 12345);
    QCOMPARE(thread.isSecure, true);
    QCOMPARE(thread.incomingPersistent, false);
    QCOMPARE(thread.getHost(), QString());
    QCOMPARE(thread.getPort(), quint16(0));
}
