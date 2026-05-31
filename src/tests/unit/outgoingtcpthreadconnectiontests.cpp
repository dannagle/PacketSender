//
// Created by Tomas Gallucci on 5/30/26.
//

#include "outgoingtcpthreadconnectiontests.h"

#include "testutils.h"
#include "testdoubles/outgoingtchpthreadtestdouble.h"
#include "testdoubles/MockSslSocket.h"

void OutgoingTcpThreadConnectionTests::testHandleOutgoingPlainTCP_callsConnectToHost()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->shouldCallConnectToHost = false;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QVERIFY(thread.callHandleOutgoingPlainTCP());

    QCOMPARE(mockSock->connectToHostCallCount, 1);
    QCOMPARE(mockSock->lastConnectedToHostName, "127.0.0.1");  // or whatever default
    QCOMPARE(mockSock->lastConnectedToPort, 666);
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingPlainTCP_emitsSuccess()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->shouldCallConnectToHost = false;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy connectionStatusSpy(&thread,  &BaseTcpThread::connectionStatus);
    QVERIFY(thread.callHandleOutgoingPlainTCP());

    QCOMPARE(connectionStatusSpy.last().at(0).value<QString>(), "Connected");
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingPlainTCP_callsHandleConnectionFailure()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->shouldCallConnectToHost = false;
    mockSock->makeWaitForConnectedReturnFalse = true;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QVERIFY(!thread.callHandleOutgoingPlainTCP());
    QVERIFY(thread.wasMethodCalled("handleConnectionFailure"));
}

// handleConnectionFailure() tests
void OutgoingTcpThreadConnectionTests::testHandleConnectionFailure_emitsConnectionStatus_CouldNotConnect()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callHandleConnectionFailure();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.last().at(0).value<QString>(), "Could not connect.");
}

void OutgoingTcpThreadConnectionTests::testHandleConnectionFailure_emitsErrorMessage()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    auto p = TestUtils::createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callHandleConnectionFailure();
    QCOMPARE(errorMessageSpy.count(), 1);

    const QString expectedErrorMessage = "Could not connect to " + p.toIP + ":" + QString::number(p.port);
    QCOMPARE(errorMessageSpy.last().at(0).value<QString>(), expectedErrorMessage);
}

void OutgoingTcpThreadConnectionTests::testHandleConnectionFailure_emitsPacketSent()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    auto p = TestUtils::createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callHandleConnectionFailure();
    QCOMPARE(packetSentSpy.count(), 1);

    p.errorString = "Could not connect";
    QCOMPARE(packetSentSpy.last().at(0).value<Packet>(), p);
}
