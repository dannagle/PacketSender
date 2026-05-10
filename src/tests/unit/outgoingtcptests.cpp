//
// Created by Tomas Gallucci on 5/9/26.
//

#include <QtTest/QTest.h>

#include <utility>

#include "outgoingtcptests.h"
#include "../../outgoingtcpthread.h"
#include "../../packet.h"
#include "testdoubles/MockSslSocket.h"
#include "testdoubles/outgoingtchpthreadtestdouble.h"


// defaults are "127.0.0.1" and 9999 respectfully
Packet OutgoingTcpTests::createPacketForTest(const QString& address, unsigned int port)
{
    Packet p;

    p.toIP = address;
    p.port = port;

    return p;
}

void OutgoingTcpTests::testConstructor_throwsIfPacketToSendPortIsNotSet()
{
    Packet p = createPacketForTest(OutgoingTcpTests::DEFAULT_ADDRESS, 0);

    try
    {
        OutgoingTcpThread thread = OutgoingTcpThread(new QSslSocket(), p);
        QFAIL("invalid_argument exception was not thrown");
    } catch (std::invalid_argument& e)
    {
        QCOMPARE(e.what(), "OutgoingTcpThread: packetToSend.port must be set to a positive integer value");
    }
}

void OutgoingTcpTests::testConstructor_throwsIfPacketToSendAddressIsNotSet()
{
    Packet p = createPacketForTest("");

    try
    {
        OutgoingTcpThread thread = OutgoingTcpThread(new QSslSocket(), p);
        QFAIL("invalid_argument exception was not thrown");
    } catch (std::invalid_argument& e)
    {
        QCOMPARE(e.what(), "OutgoingTcpThread: packetToSend.toIP cannot be empty");
    }
}

void OutgoingTcpTests::testGetDestinationAddress()
{
    QString destinationAddress = "foo bar baz";
    Packet p = createPacketForTest(destinationAddress);

    OutgoingTcpThread thread = OutgoingTcpThread(new QSslSocket(), p);
    QCOMPARE(thread.getDestinationAddress(), destinationAddress);
}

void OutgoingTcpTests::testGetDestinationPort()
{
    unsigned int port = 666;
    Packet p = createPacketForTest(OutgoingTcpTests::DEFAULT_ADDRESS, port);

    OutgoingTcpThread thread = OutgoingTcpThread(new QSslSocket(), p);
    QCOMPARE(thread.getDestinationPort(), port);
}

// isValid() tests
void OutgoingTcpTests::testIsValid_returnsFalseWhenSendPacketDotToIpIsEmptyString()
{
    // taking advantage of createPacketForTest defaults
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(new QSslSocket(), createPacketForTest());

    Packet p = thread.getSendPacketByReference();
    p.port = 0;

    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpTests::testIsValid_returnsFalseWhenSendPacketDotPortIsZero()
{
    // taking advantage of createPacketForTest defaults
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(new QSslSocket(), createPacketForTest());

    Packet p = thread.getSendPacketByReference();
    p.toIP = "";

    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpTests::testIsValid_returnsFalseWhenSocketHasNotBeenConnectedToHost()
{
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(new QSslSocket(), createPacketForTest());
    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpTests::testIsValid_returnsTrueWithValidPacketAndSocket()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);        // important
    mockSock->setIsValid(true);            // if you have this

    Packet validPacket = createPacketForTest("127.0.0.1", 9999);

    OutgoingTcpThreadTestDouble thread(mockSock, validPacket);

    QCOMPARE(thread.isValid(), true);
}


