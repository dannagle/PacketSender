//
// Created by Tomas Gallucci on 6/17/26.
//

#include "incomingtcpconnectiontests.h"

#include "incomingtcpthread.h"
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

void IncomingTcpConnectionTests::testSend_throwsRuntimeException()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketWithSocketDescriptorOtherThan0());
    auto connection = std::make_unique<IncomingTcpConnectionTestDouble>();

    const auto p = TestUtils::createPacketForTest();

    connection->setThread(std::move(thread));

    try
    {
        connection->send(p);
        QFAIL("send should have thrown");
    } catch(std::runtime_error& e)
    {
        // Because we're using a TestDouble, the error message will read
        // "Unsupported Operation: IncomingTcpConnectionTestDouble cannot send Packet"
        // but production code will read "Unsupported Operation: IncomingTcpConnection\\w* cannot send Packet"
        QRegularExpression re("Unsupported Operation: IncomingTcpConnection\\w* cannot send Packet");
        auto fromString = QString::fromStdString(e.what());
        QVERIFY(re.match(fromString).hasMatch());
    }
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
