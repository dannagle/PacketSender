//
// Created by Tomas Gallucci on 5/19/26.
//

#include "outgoingtcpthreadpersistentconnectionlooptests.h"

#include "testutils.h"
#include "testdoubles/outgoingtchpthreadtestdouble.h"

void OutgoingTcpThreadPersistentConnectionLoopTests::testShouldContinuePersistentLoop_returnsTrueWhenAllConditionsMet()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    QVERIFY(thread.callShouldContinuePersistentConnectionLoop());
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testShouldContinuePersistentLoop_returnsFalseWhenInterruptionRequested()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.stop();

    QVERIFY(!thread.callShouldContinuePersistentConnectionLoop());
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testShouldContinuePersistentLoop_returnsFalseWhenNoSocket()
{
    OutgoingTcpThreadTestDouble thread(new MockSslSocket(), TestUtils::createPacketForTest());
    thread.setSocketForTest(nullptr);   // force no socket

    QVERIFY(!thread.callShouldContinuePersistentConnectionLoop());
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testShouldContinuePersistentLoop_returnsFalseWhenSocketNotConnected()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockState(QAbstractSocket::UnconnectedState);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QVERIFY(!thread.callShouldContinuePersistentConnectionLoop());
}

// shouldStopPersistentLoop() tests
void OutgoingTcpThreadPersistentConnectionLoopTests::testShouldStopPersistentLoop_returnsFalseWhenStopHasBeenCalled()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QVERIFY(!thread.callShouldStopPersistentConnectionLoop());
}

    void OutgoingTcpThreadPersistentConnectionLoopTests::testShouldStopPersistentLoop_returnsFalseWhenStopHasNotBeenCalled()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.stop();
    QVERIFY(thread.callShouldStopPersistentConnectionLoop());
}

// handlePersistentIdleCase() tests
void OutgoingTcpThreadPersistentConnectionLoopTests::testHandlePersistentIdleCase_emitsWhenThresholdPassed()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy statusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callHandlePersistentIdleCase(100);   // small threshold

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(statusSpy.first().first().toString(), "Connected and idle.");
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testHandlePersistentIdleCase_doesNotEmitTooSoon()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    QSignalSpy statusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callHandlePersistentIdleCase(1000);  // first call
    statusSpy.clear();

    thread.callHandlePersistentIdleCase(1000);  // immediate second call
    QCOMPARE(statusSpy.count(), 0);   // should not emit again
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testHandlePersistentIdleCase_respectsCustomThreshold()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy statusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callHandlePersistentIdleCase(50);   // very short threshold

    QCOMPARE(statusSpy.count(), 1);
}

// persistentConnectionLoop() tests
void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_exitsImmediatelyOnInterruption()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy statusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.stop();
    thread.callPersistentConnectionLoop();

    // Should exit immediately without entering main loop
    QCOMPARE(statusSpy.count(), 0);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_callsShouldContinuePersistentLoop()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.callPersistentConnectionLoop();
    QCOMPARE(thread.shouldContinuePersistentConnectionLoopCallCount, 2);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_callsShouldStopPersistentLoop()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.callPersistentConnectionLoop();

    constexpr int beforeLoopCallCount = 1;
    constexpr int insideLoopCallCount = 1;
    QCOMPARE(thread.shouldStopPersistentConnectionLoopCallCount, beforeLoopCallCount + insideLoopCallCount);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_callsIdleHandlerWhenNoData()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    Packet p = TestUtils::createPacketForTest();
    p.hexString.clear();
    p.persistent = true;

    OutgoingTcpThreadTestDouble thread(mockSock, p);

    QSignalSpy statusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callPersistentConnectionLoop();
    TestUtils::debugSpy(statusSpy);
    QVERIFY(statusSpy.contains(QVariantList{"Connected and idle."}));
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_skipsIdleWhenHasDataToSend()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    Packet p = TestUtils::createPacketForTest(); // has hexString
    p.persistent = true;

    OutgoingTcpThreadTestDouble thread(mockSock, p);

    thread.callPersistentConnectionLoop();

    // Add a counter in test double for how many times idle handler was called
    QCOMPARE(thread.handlePersistentIdleCaseCallCount, 0);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_doesNotEnterLoopIfAlreadyStopped()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.stop();  // stop before calling

    thread.callPersistentConnectionLoop();
    QCOMPARE(thread.shouldContinuePersistentConnectionLoopCallCount, 0);   // never entered while()
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_exitsWhenSocketDisconnects()
{
    auto* mockSock = TestUtils::createMockSocketForTest();

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createIdlePacketForTest());

    // Simulate socket becoming disconnected during loop
    mockSock->setMockState(QAbstractSocket::UnconnectedState);

    thread.callPersistentConnectionLoop();
    QVERIFY(thread.loopExitedCleanly());
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_emitsIdleStatusOnlyEveryTwoSeconds()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    Packet idlePacket = TestUtils::createIdlePacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSock, idlePacket);
    thread.forceExitAfterNIterations = 5;   // run several times

    QSignalSpy statusSpy(&thread, &BaseTcpThread::connectionStatus);
    thread.callPersistentConnectionLoop();

    // Should emit at most a couple of times even after 5 iterations
    QVERIFY(statusSpy.count() <= 3);
}
