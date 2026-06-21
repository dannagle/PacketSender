//
// Created by Tomas Gallucci on 6/20/26.
//

#include "outgoingtcpconnectiontests.h"

#include "../../connections/outgoingtcpconnection.h"
#include "../utils/testutils.h"
#include "testdoubles/connections/outgoingtcpconnectiontestdouble.h"

void OutgoingTcpConnectionTests::test_send_threadDoesNotExist()
{
    auto connection = OutgoingTcpConnectionTestDouble();

    const auto packet = TestUtils::createPacketForTest();
    connection.send(packet);

    // Wait up to 500ms for the RUN() call to be recorded
    QTRY_VERIFY_WITH_TIMEOUT(
        connection.getThread().wasMethodCalled(CallTracker::RUN()),
        500
    );

    qDebug() << "Main thread ID:" << QThread::currentThreadId();
    qDebug() << "Connection thread ID:" << connection.getThread().currentThreadId();
    qDebug() << "connection->getThread().wasMethodCalled(CallTracker::RUN()): " << connection.getThread().wasMethodCalled(CallTracker::RUN());

}

void OutgoingTcpConnectionTests::test_send_replacesExistingThread()
{
    auto packet = TestUtils::createPacketForTest();
    OutgoingTcpConnectionTestDouble connection;

    // 1. Give it an initial thread
    auto initialThread = std::make_unique<OutgoingTcpThreadTestDouble>(packet, &connection);
    const auto initialThreadId = initialThread->id();

    // Initial Spy threads
    QSignalSpy shutdownSpy(initialThread.get(),
                          &OutgoingTcpThreadTestDouble::outgoingThreadTestDoubleAboutToShutdown);
    QSignalSpy destroyedSpy(initialThread.get(),
                            &OutgoingTcpThreadTestDouble::threadTestDoubleDestructorCalled);

    connection.setThread(std::move(initialThread));

    QVERIFY(connection.getThread().wasMethodCalled(CallTracker::RUN()) == false); // hasn't started yet

    // 2. Call send() — this should shutdown the old thread and create a new one
    connection.send(packet);

    // 3. Verify the old thread was cleaned up
    // Wait up to 500ms for the RUN() call to be recorded
    QTRY_VERIFY_WITH_TIMEOUT(shutdownSpy.count()  == 1, 500);
    QTRY_VERIFY_WITH_TIMEOUT(destroyedSpy.count() == 1, 500);

    // 4. Verify we now have a *new* thread
    OutgoingTcpThreadTestDouble& newThread = connection.getThread();
    QTRY_VERIFY_WITH_TIMEOUT(newThread.getThreadIdCapturedInRun() != nullptr, 500);

    QVERIFY2(newThread.id() != initialThreadId, "send() did not replace the old thread with a new one");
}
