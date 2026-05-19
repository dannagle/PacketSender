//
// Created by Tomas Gallucci on 5/9/26.
//

#include <QtTest/QTest.h>

#include <utility>

#include "outgoingtcpthreadtests.h"
#include "../../outgoingtcpthread.h"
#include "../../packet.h"
#include "../../../../../../../../../opt/homebrew/lib/QtTest.framework/Headers/QSignalSpy"
#include "testdoubles/MockSslSocket.h"
#include "testdoubles/outgoingtchpthreadtestdouble.h"


// defaults are "127.0.0.1" and 9999 respectfully
Packet OutgoingTcpThreadTests::createPacketForTest(const QString& address, unsigned int port)
{
    Packet p;

    p.toIP = address;
    p.port = port;
    p.hexString = "AA BB CC DD";

    return p;
}

void OutgoingTcpThreadTests::testConstructor_throwsIfPacketToSendPortIsNotSet()
{
    Packet p = createPacketForTest(OutgoingTcpThreadTests::DEFAULT_ADDRESS, 0);

    try
    {
        OutgoingTcpThread thread = OutgoingTcpThread(p);
        QFAIL("invalid_argument exception was not thrown");
    } catch (std::invalid_argument& e)
    {
        QCOMPARE(e.what(), "OutgoingTcpThread: packetToSend.port must be set to a positive integer value");
    }
}

void OutgoingTcpThreadTests::testConstructor_throwsIfPacketToSendAddressIsNotSet()
{
    Packet p = createPacketForTest("");

    try
    {
        OutgoingTcpThread thread = OutgoingTcpThread(p);
        QFAIL("invalid_argument exception was not thrown");
    } catch (std::invalid_argument& e)
    {
        QCOMPARE(e.what(), "OutgoingTcpThread: packetToSend.toIP cannot be empty");
    }
}

void OutgoingTcpThreadTests::testGetDestinationAddress()
{
    QString destinationAddress = "foo bar baz";
    Packet p = createPacketForTest(destinationAddress);

    OutgoingTcpThread thread = OutgoingTcpThread(p);
    QCOMPARE(thread.getDestinationAddress(), destinationAddress);
}

void OutgoingTcpThreadTests::testGetDestinationPort()
{
    unsigned int port = 666;
    Packet p = createPacketForTest(OutgoingTcpThreadTests::DEFAULT_ADDRESS, port);

    OutgoingTcpThread thread = OutgoingTcpThread(p);
    QCOMPARE(thread.getDestinationPort(), port);
}

// isValid() tests
void OutgoingTcpThreadTests::testIsValid_returnsFalseWhenSendPacketDotToIpIsEmptyString()
{
    // taking advantage of createPacketForTest defaults
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(new QSslSocket(), createPacketForTest());

    Packet p = thread.getSendPacketByReference();
    p.port = 0;

    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpThreadTests::testIsValid_returnsFalseWhenSendPacketDotPortIsZero()
{
    // taking advantage of createPacketForTest defaults
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(createPacketForTest());

    Packet p = thread.getSendPacketByReference();
    p.toIP = "";

    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpThreadTests::testIsValid_returnsFalseWhenSocketHasNotBeenConnectedToHost()
{
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(createPacketForTest());
    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpThreadTests::testIsValid_returnsTrueWithValidPacketAndSocket()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    Packet validPacket = createPacketForTest("127.0.0.1", 9999);

    OutgoingTcpThreadTestDouble thread(mockSock, validPacket);
    QCOMPARE(thread.isValid(), true);
}

// preparePacket() tests
void OutgoingTcpThreadTests::testPreparePacket()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    Packet validPassedInPacket = createPacketForTest("127.0.0.1", 9999);

    OutgoingTcpThreadTestDouble thread(mockSock, validPassedInPacket);
    thread.callPrepareOutgoingSendPacket();

    const Packet sendPreparedPacket = thread.getSendPacketByReference();
    QCOMPARE("You", sendPreparedPacket.fromIP);
    QVERIFY(sendPreparedPacket.timestamp.isValid());
    QCOMPARE(sendPreparedPacket.timestamp.toString(DATETIMEFORMAT), sendPreparedPacket.name);
}

// closeConnection() tests
void OutgoingTcpThreadTests::testCloseConnection_SocketIsConnected_DisconnectFromHostCalled()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    Packet p = createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);

    thread.callCloseConnection();
    QVERIFY(mockSock != nullptr);
    QCOMPARE(mockSock->disconnectFromHostCallCount, 1);
}

void OutgoingTcpThreadTests::testCloseConnection_SocketIsClosing_DisconnectFromHostCalled()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ClosingState);

    Packet p = createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);

    thread.callCloseConnection();
    QCOMPARE(mockSock->disconnectFromHostCallCount, 1);
}

void OutgoingTcpThreadTests::testCloseConnection_CloseCalled()
{
    auto *mockSock = new MockSslSocket();
    OutgoingTcpThreadTestDouble thread(mockSock, createPacketForTest());
    thread.callCloseConnection();
    QCOMPARE(mockSock->closeCallCount, 1);
}

void OutgoingTcpThreadTests::testCloseConnection_emitsConnectionStatus_Disconnected()
{
    auto *mockSock = new MockSslSocket();

    OutgoingTcpThreadTestDouble thread(mockSock, createPacketForTest());
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);
    thread.callCloseConnection();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().first().value<QString>(), "Disconnected");
}
