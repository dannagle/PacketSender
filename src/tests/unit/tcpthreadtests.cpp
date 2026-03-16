//
// Created by Tomas Gallucci on 3/6/26.
//

#include <QtTest/QTest.h>
#include "tcpthreadtests.h"

#include "testdoubles/testtcpthreadclass.h"

#include "packet.h"

void TcpThreadTests::testIncomingConstructorBasic()
{
    // Use invalid descriptor (real ones are positive; -1 is common sentinel)
    const TestTcpThreadClass thread(-1, /*isSecure*/ false, /*isPersistent*/ true);

    QVERIFY(thread.getClientConnection() != nullptr);
    QVERIFY(qobject_cast<QSslSocket*>(thread.getClientConnection()) != nullptr);

    QCOMPARE(thread.getSocketDescriptor(), -1);
    QCOMPARE(thread.isSecure, false);
    QCOMPARE(thread.incomingPersistent, true);
    QCOMPARE(thread.getHost(), QString());
    QCOMPARE(thread.getPort(), quint16(0));

    // Optional: check other defaults
    QCOMPARE(thread.sendFlag, false);
    QCOMPARE(thread.consoleMode, false);
    QCOMPARE(thread.getManagedByConnection(), true);
}

void TcpThreadTests::testIncomingConstructorWithSecureFlag()
{
    const TestTcpThreadClass thread(12345, /*isSecure*/ true, /*isPersistent*/ false);

    QVERIFY(thread.getClientConnection() != nullptr);
    QVERIFY(qobject_cast<QSslSocket*>(thread.getClientConnection()) != nullptr);

    QCOMPARE(thread.getSocketDescriptor(), 12345);
    QCOMPARE(thread.isSecure, true);
    QCOMPARE(thread.incomingPersistent, false);
    QCOMPARE(thread.getHost(), QString());
    QCOMPARE(thread.getPort(), quint16(0));
}

//////////////////////////////////////////////////////////////////////
///                 DETERMINE IP MODE TESTS                 /////////
////////////////////////////////////////////////////////////////////

// Test 1: Both host and sendPacket.toIP are IPv4 → returns IPv4Protocol
void TcpThreadTests::testGetIPConnectionProtocol_bothIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("127.0.0.1", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 2: Both are IPv6 → returns IPv6Protocol
void TcpThreadTests::testGetIPConnectionProtocol_bothIPv6_returnsIPv6()
{
    TestTcpThreadClass thread("::1", 80, Packet());
    thread.setSendPacketToIp("::1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);
}

// Test 3: Host IPv4, sendPacket IPv6 → returns IPv6Protocol (prefers sendPacket.toIP)
void TcpThreadTests::testGetIPConnectionProtocol_hostIPv4_packetIPv6_returnsPacketValue()
{
    TestTcpThreadClass thread("127.0.0.1", 80, Packet());
    thread.setSendPacketToIp("::1");  // mismatch
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);  // prefers sendPacket.toIP
}

// Test 4: Host IPv6, sendPacket IPv4 → returns IPv4Protocol (prefers sendPacket.toIP)
void TcpThreadTests::testGetIPConnectionProtocol_hostIPv6_packetIPv4_returnsPacketValue()
{
    TestTcpThreadClass thread("::1", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");  // mismatch
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);  // prefers sendPacket.toIP
}

// Test 5: Host IPv4-mapped IPv6, sendPacket IPv4 → returns IPv6Protocol
void TcpThreadTests::testGetIPConnectionProtocol_hostMappedIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("::ffff:127.0.0.1", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 6: Host empty, sendPacket IPv4 → returns IPv4Protocol
void TcpThreadTests::testGetIPConnectionProtocol_hostEmpty_packetIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 7: Host empty, sendPacket IPv6 → returns IPv6Protocol
void TcpThreadTests::testGetIPConnectionProtocol_hostEmpty_packetIPv6_returnsIPv6()
{
    TestTcpThreadClass thread("", 80, Packet());
    thread.setSendPacketToIp("::1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);
}

// Test 8: Both empty → returns IPv4Protocol (default/fallback)
void TcpThreadTests::testGetIPConnectionProtocol_bothEmpty_returnsIPv4()
{
    TestTcpThreadClass thread("", 80, Packet());
    thread.setSendPacketToIp("");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 9: Host invalid string, sendPacket IPv4 → returns IPv4Protocol (fallback)
void TcpThreadTests::testGetIPConnectionProtocol_hostInvalid_packetIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("invalid-host-string", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}
