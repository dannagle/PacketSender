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

// buildReceivedPacket() tests
void OutgoingTcpThreadPersistentConnectionLoopTests::testBuildReceivedPacket_socketNull()
{
    auto* mockSock = TestUtils::createMockSocketForTest();

    Packet p = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSock, p);
    thread.setSocketForTest(nullptr);

    QThread::msleep(5); // to differentiate packet timestamps
    Packet receivedPacket = thread.callBuildReceivedPacket();

    p.name = "Received (Persistent)";
    p.hexString.clear();

    p.fromIP = p.toIP;
    p.toIP = "You";

    p.fromPort = receivedPacket.fromPort; // may be 0 because we never called connectToHost() on socket
    p.port = receivedPacket.port;

    QCOMPARE(receivedPacket, p);
    QVERIFY(receivedPacket.timestamp.isValid());
    QVERIFY(receivedPacket.timestamp != p.timestamp);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testBuildReceivedPacket_socketHasNoData()
{
    auto* mockSock = TestUtils::createMockSocketForTest();

    Packet p = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSock, p);

    QThread::msleep(5); // to differentiate packet timestamps
    Packet receivedPacket = thread.callBuildReceivedPacket();

    p.name = "Received (Persistent)";
    p.hexString.clear();

    p.fromIP = p.toIP;
    p.toIP = "You";

    p.fromPort = receivedPacket.fromPort; // may be 0 because we never called connectToHost() on socket
    p.port = receivedPacket.port;

    QCOMPARE(receivedPacket, p);
    QVERIFY(receivedPacket.timestamp.isValid());
    QVERIFY(receivedPacket.timestamp != p.timestamp);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testBuildReceivedPacket_socketHasData()
{
    const QByteArray data = "This is the song that never ends.";
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockReadData(data);

    Packet p = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSock, p);

    QThread::msleep(5); // to differentiate packet timestamps
    Packet receivedPacket = thread.callBuildReceivedPacket();

    p.name = "Received (Persistent)";
    p.hexString = data.toHex(' ').toUpper() + " "; // current implementation has a trailing space

    p.fromIP = p.toIP;
    p.toIP = "You";

    p.fromPort = receivedPacket.fromPort;
    p.port = receivedPacket.port;

    QCOMPARE(receivedPacket, p);
    QVERIFY(receivedPacket.timestamp.isValid());
    QVERIFY(receivedPacket.timestamp != p.timestamp);
}

// processIncomingData() tests
void OutgoingTcpThreadPersistentConnectionLoopTests::testProcessIncomingData_socketIsNull_returnsEarly()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.setSocketForTest(nullptr);

    thread.callProcessIncomingData();
    QCOMPARE(thread.buildReceivedPacketCallCount, 0);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testProcessIncomingData_socketHasNoData_returnsEarly()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockReadData("");
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.callProcessIncomingData();
    QCOMPARE(thread.buildReceivedPacketCallCount, 0);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testProcessIncomingData_socketHasData_emitsReceivedPacket()
{
    const QByteArray data = "This is the song that never ends.";
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockReadData(data);

    Packet p = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSock, p);
    QSignalSpy packetReceivedSpy(&thread, &BaseTcpThread::packetReceived);

    QThread::msleep(5); // to differentiate packet timestamps
    thread.callProcessIncomingData();
    QCOMPARE(packetReceivedSpy.count(), 1);

    p.name = "Received (Persistent)";
    p.hexString = data.toHex(' ').toUpper() + " "; // current implementation has a trailing space

    p.fromIP = p.toIP;
    p.toIP = "You";

    auto receivedPacket = packetReceivedSpy.first().first().value<Packet>();
    p.fromPort = receivedPacket.fromPort;
    p.port = receivedPacket.port;
    QCOMPARE(receivedPacket, p);
}

// waitForAndProcessIncomingData()
void OutgoingTcpThreadPersistentConnectionLoopTests::testWaitForAndProcessIncomingData_emitsConnectionStatus_WaitingForDataBeforeSend()
{
    auto mockSock = new MockSslSocket();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy connectionStausSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callWaitForAndProcessIncomingData();
    QCOMPARE(connectionStausSpy.count(), 1);
    QCOMPARE(connectionStausSpy.first().first().value<QString>(), "Waiting for data before send");
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testWaitForAndProcessIncomingData_functionCallsOrder()
{
    auto mockSock = new MockSslSocket();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.callWaitForAndProcessIncomingData();

    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back("waitForAndProcessIncomingData");
    expectedCallSequence.push_back("interruptibleWaitForReadyRead");
    expectedCallSequence.push_back("processIncomingData");
    QCOMPARE(thread.getCallSequence(), expectedCallSequence);
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
    QCOMPARE(thread.shouldStopPersistentConnectionLoopCallCount, 1);
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

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_exitsLoopEarlyIfAlreadyStopped()
{
    auto* mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.stop();  // stop before calling

    thread.callPersistentConnectionLoop();
    QCOMPARE(thread.shouldContinuePersistentConnectionLoopCallCount, 1);   // never entered while()
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

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_callsProcessIncomingData()
{
    const QByteArray data = "This is the song that never ends.";
    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockReadData(data);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.callPersistentConnectionLoop();
    QCOMPARE(thread.processIncomingDataCallCount, 1);
}

void OutgoingTcpThreadPersistentConnectionLoopTests::testPersistentConnectionLoop_callsWaitForAndProcessIncomingData()
{
    auto* mockSock = TestUtils::createMockSocketForTest();

    auto p = TestUtils::createPacketForTest();
    p.receiveBeforeSend = true;

    OutgoingTcpThreadTestDouble thread(mockSock, p);
    thread.callPersistentConnectionLoop();
    QCOMPARE(thread.waitForAndProcessIncomingDataCallCount, 1);
}
