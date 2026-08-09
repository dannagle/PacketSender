//
// Created by Tomas Gallucci on 5/19/26.
//

#include "singlesendoutgoingtcpthreadtests.h"

#include "settingnames.h"
#include "../../packet.h"
#include "testdoubles/MockSslSocket.h"
#include "testdoubles/outgoingtcpthreadtestdouble.h"
#include "utils/testutils.h"

void SingleSendOutgoingTcpThreadTests::testRun_SocketSuccessfullyConnected_emitsConnectionStatus_Connected()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    Packet packet = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 3);
    QCOMPARE(connectionStatusSpy.at(0).first().toString(), "Connected");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketSuccessfullyConnected_emitsConnectionStatus_SendingData()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    Packet packet = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 3);
    QCOMPARE(connectionStatusSpy.at(1).first().toString(), "Sending data: " + packet.asciiString());
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketSuccessfullyConnected_emitsConnectionStatus_Disconnected()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    Packet packet = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 3);
    QCOMPARE(connectionStatusSpy.at(2).first().toString(), "Disconnected");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_emitsConnectionStatus_CouldNotConnect()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.at(0).first().toString(), "Could not connect.");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_packetErrorMessageUpdated()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = TestUtils::createPacketForTest();
    QVERIFY(packet.errorString.isEmpty());

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);

    thread.callRun();
    QCOMPARE(thread.getSendPacketByReference().errorString, "Could not connect");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_emitsPacketSent()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = TestUtils::createPacketForTest();
    QVERIFY(packet.errorString.isEmpty());


    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy packetSentSpy(&thread,
                         QOverload<const Packet&>::of(&BaseTcpThread::packetSent));
    thread.callRun();

    auto sendPacketFromInsideThread = thread.getSendPacketByReference();
    packet.errorString = "Could not connect";
    QCOMPARE(sendPacketFromInsideThread, packet);
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_emitsErrorMessage()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = TestUtils::createPacketForTest();
    QVERIFY(packet.errorString.isEmpty());

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    thread.callRun();

    QCOMPARE(errorMessageSpy.count(), 1);

    QString expectedErrorMessage = "Could not connect to " + packet.toIP + ":" + QString::number(packet.port);
    QCOMPARE(errorMessageSpy.at(0).first().toString(), expectedErrorMessage);
}

void SingleSendOutgoingTcpThreadTests::testRun_successPath_callsMethodsInCorrectOrder()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    Packet packet = TestUtils::createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);

    thread.callRun();

    const auto& callSequence = thread.getCallSequence();

    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back(CallTracker::HANDLE_OUTGOING_PLAIN_TCP());
    expectedCallSequence.push_back(CallTracker::PREPARE_OUTGOING_PACKET());
    expectedCallSequence.push_back(CallTracker::SEND_OUTGOING_PACKET());
    expectedCallSequence.push_back(CallTracker::OUTGOINGTCPTHREAD_ISVALIDFORSENDING());
    expectedCallSequence.push_back(CallTracker::PROCESS_INCOMING_DATA());
    expectedCallSequence.push_back(CallTracker::CLOSE_CONNECTION());
    QCOMPARE(callSequence, expectedCallSequence);
}

void SingleSendOutgoingTcpThreadTests::testRun_respectsDelayAfterConnect()
{
    auto mockSocket = TestUtils::createMockSocketForTest();
    Packet packet = TestUtils::createPacketForTest();
    packet.delayAfterConnect = 100;   // 100ms

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);

    thread.callRun();
    QCOMPARE(thread.sleepCallCount, 1);

    auto callSequence = thread.getCallSequence();

    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back(CallTracker::HANDLE_OUTGOING_PLAIN_TCP());
    expectedCallSequence.push_back("usleep 100000 usecs");
    expectedCallSequence.push_back(CallTracker::PREPARE_OUTGOING_PACKET());
    expectedCallSequence.push_back(CallTracker::SEND_OUTGOING_PACKET());
    expectedCallSequence.push_back(CallTracker::OUTGOINGTCPTHREAD_ISVALIDFORSENDING());
    expectedCallSequence.push_back(CallTracker::PROCESS_INCOMING_DATA());
    expectedCallSequence.push_back(CallTracker::CLOSE_CONNECTION());
    QCOMPARE(callSequence, expectedCallSequence);
}

void SingleSendOutgoingTcpThreadTests::testSingleShot_sendsSmartResponse()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockReadData("DE AD BE EF");

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.setConsoleMode(false);

    // Setup smart response
    QSettings &settings = getSettings();
    settings.setValue(SMART_RESPONSES_ENABLED, true);
    settings.setValue("responseEnableCheck1", true);
    settings.setValue("responseIfEdit1", "44 45 20 41 44 20 42 45 20 45 46");
    settings.setValue("responseReplyEdit1", "CA FE BA BE");
    settings.setValue("matchMethodBox1", "Exact Match");
    settings.setValue("responseEncodingBox1", "HEX");
    settings.sync();

    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callProcessIncomingData();
    QCOMPARE(packetSentSpy.count(), 1);

    auto sent = packetSentSpy.first().first().value<Packet>();
    QCOMPARE(Packet::byteArrayToHex(sent.getByteArray()), "CA FE BA BE ");
}
