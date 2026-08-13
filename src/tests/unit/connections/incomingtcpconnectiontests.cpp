//
// Created by Tomas Gallucci on 6/17/26.
//

#include "incomingtcpconnectiontests.h"

#include "../../tcpThreads/incomingtcpthread.h"
#include "connections/incomingtcpconnection.h"
#include "testdoubles/connections/incomingtcpconnectiontestdouble.h"
#include "tests/unit/utils/testutils.h"

void IncomingTcpConnectionTests::testIsIncoming()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketWithSocketDescriptorOtherThan0());
    auto conn = std::make_unique<IncomingTcpConnectionTestDouble>();
    conn->setThread(std::move(thread));
    QCOMPARE(conn->isIncoming(), true);
}

void IncomingTcpConnectionTests::testSend_EmitsSendRequested_whenThreadIsNotNullPtr()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketWithSocketDescriptorOtherThan0());
    auto connection = std::make_unique<IncomingTcpConnectionTestDouble>();

    const auto p = TestUtils::createPacketForTest();

    connection->setThread(std::move(thread));

    QSignalSpy sendRequestedSpy(connection.get(), &IncomingTcpConnectionTestDouble::sendRequested);
    connection->send(p);

    QTRY_COMPARE_EQ_WITH_TIMEOUT(sendRequestedSpy.count(), 1, 1000);
}

void IncomingTcpConnectionTests::testSend_EmitsErrorOccurred_whenThreadIsNullPtr()
{
    // auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketWithSocketDescriptorOtherThan0());
    auto connection = std::make_unique<IncomingTcpConnectionTestDouble>();

    const auto p = TestUtils::createPacketForTest();

    // connection->setThread(std::move(thread));

    QSignalSpy sendRequestedSpy(connection.get(), &IncomingTcpConnectionTestDouble::sendRequested);
    QSignalSpy errorOccurredSpy(connection.get(), &IncomingTcpConnectionTestDouble::errorOccurred);

    connection->send(p);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(errorOccurredSpy.count(), 1, 1000);
    QCOMPARE(errorOccurredSpy.first().first(), "No active thread to send on");

    QTRY_COMPARE_EQ_WITH_TIMEOUT(sendRequestedSpy.count(), 0, 1000);
}

void IncomingTcpConnectionTests::test_receiveData_threadDoesNotExist()
{
    constexpr bool isSecure = false;
    constexpr bool isPersistent = false;
    auto connection = IncomingTcpConnectionTestDouble();

    connection.receiveData(555, isSecure, isPersistent);

    // Wait up to 500ms for the RUN() call to be recorded
    QTRY_VERIFY_WITH_TIMEOUT(
        connection.getThread().wasMethodCalled(CallTracker::RUN()),
        500
    );

    qDebug() << "Main thread ID:" << QThread::currentThreadId();
    qDebug() << "Connection thread ID:" << connection.getThread().currentThreadId();
    qDebug() << "connection->getThread().wasMethodCalled(CallTracker::RUN()): " << connection.getThread().wasMethodCalled(CallTracker::RUN());
}

void IncomingTcpConnectionTests::test_receiveData_replacesExistingThread()
{
    constexpr bool isSecure = false;
    constexpr bool isPersistent = false;

    auto packet = TestUtils::createPacketForTest();
    IncomingTcpConnectionTestDouble connection;

    // 1. Give it an initial thread
    auto initialThread = std::make_unique<IncomingTcpThreadTestDouble>(666, &connection);
    const auto initialThreadId = initialThread->id();

    // Initial Spy threads
    QSignalSpy shutdownSpy(initialThread.get(),
                          &IncomingTcpThreadTestDouble::shutdownCalled);
    QSignalSpy destroyedSpy(initialThread.get(),
                            &IncomingTcpThreadTestDouble::destructorCalled);

    connection.setThread(std::move(initialThread));

    QVERIFY(connection.getThread().wasMethodCalled(CallTracker::RUN()) == false); // hasn't started yet
    QCOMPARE(connection.getCallCount(CallTracker::SETUP_SIGNAL_CONNECTIONS()), 0);

    // 2. Call send() — this should shutdown the old thread and create a new one
    connection.receiveData(777, isSecure, isPersistent);

    // 3. Verify the old thread was cleaned up
    // Wait up to 500ms for the RUN() call to be recorded
    QTRY_VERIFY_WITH_TIMEOUT(shutdownSpy.count()  == 1, 500);
    QTRY_VERIFY_WITH_TIMEOUT(destroyedSpy.count() == 1, 500);

    // 4. Verify we now have a *new* thread
    IncomingTcpThreadTestDouble& newThread = connection.getThread();
    QTRY_VERIFY_WITH_TIMEOUT(newThread.getThreadIdCapturedInRun() != nullptr, 500);

    QVERIFY2(newThread.id() != initialThreadId, "send() did not replace the old thread with a new one");
    QCOMPARE(connection.getCallCount(CallTracker::SETUP_SIGNAL_CONNECTIONS()), 1);
}
