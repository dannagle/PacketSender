//
// Created by Tomas Gallucci on 3/6/26.
//

#include <QtTest/QTest.h>
#include "tcpthreadtests.h"

#include "testdoubles/testtcpthreadclass.h"



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
