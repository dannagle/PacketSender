//
// Created by Tomas Gallucci on 5/19/26.
//

#include <QSignalSpy>

#include "singlesendoutgoingtcpthreadtests.h"

#include "../../packet.h"
#include "testdoubles/MockSslSocket.h"
#include "testdoubles/outgoingtchpthreadtestdouble.h"

Packet createPacketForTest()
{
    Packet p;

    p.toIP = "127.0.0.1";
    p.port = 666;
    p.hexString = "AA BB CC DD";
    p.tcpOrUdp = "TCP";

    return p;
}

MockSslSocket* createMockSocketForTest()
{
    auto mockSock = new MockSslSocket();;

    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    return mockSock;
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketSuccessfullyConnected_emitsConnectionStatus_Connected()
{
    auto mockSocket = createMockSocketForTest();
    Packet packet = createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 3);
    QCOMPARE(connectionStatusSpy.at(0).first().toString(), "Connected");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketSuccessfullyConnected_emitsConnectionStatus_SendingData()
{
    auto mockSocket = createMockSocketForTest();
    Packet packet = createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 3);
    QCOMPARE(connectionStatusSpy.at(1).first().toString(), "Sending data: " + packet.asciiString());
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketSuccessfullyConnected_emitsConnectionStatus_Disconnected()
{
    auto mockSocket = createMockSocketForTest();
    Packet packet = createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 3);
    QCOMPARE(connectionStatusSpy.at(2).first().toString(), "Disconnected");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_emitsConnectionStatus_CouldNotConnect()
{
    auto mockSocket = createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.at(0).first().toString(), "Could not connect.");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_packetErrorMessageUpdated()
{
    auto mockSocket = createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = createPacketForTest();
    QVERIFY(packet.errorString.isEmpty());

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);

    thread.callRun();
    QCOMPARE(thread.getSendPacketByReference().errorString, "Could not connect");
}

void SingleSendOutgoingTcpThreadTests::testRun_SocketNotConnected_emitsPacketSent()
{
    auto mockSocket = createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = createPacketForTest();
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
    auto mockSocket = createMockSocketForTest();
    mockSocket->makeWaitForConnectedReturnFalse = true;

    Packet packet = createPacketForTest();
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
    auto mockSocket = createMockSocketForTest();
    Packet packet = createPacketForTest();

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);

    thread.callRun();

    const auto& callSequence = thread.getCallSequence();

    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back("prepareOutgoingPacket");
    expectedCallSequence.push_back("sendOutgoingPacket");
    expectedCallSequence.push_back("closeConnection");
    QCOMPARE(callSequence, expectedCallSequence);
}

void SingleSendOutgoingTcpThreadTests::testRun_respectsDelayAfterConnect()
{
    auto mockSocket = createMockSocketForTest();
    Packet packet = createPacketForTest();
    packet.delayAfterConnect = 100;   // 100ms

    OutgoingTcpThreadTestDouble thread(mockSocket, packet);

    thread.callRun();
    QCOMPARE(thread.sleepCallCount, 1);

    auto callSequence = thread.getCallSequence();

    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back("usleep 100000 usecs");
    expectedCallSequence.push_back("prepareOutgoingPacket");
    expectedCallSequence.push_back("sendOutgoingPacket");
    expectedCallSequence.push_back("closeConnection");
    QCOMPARE(callSequence, expectedCallSequence);
}
