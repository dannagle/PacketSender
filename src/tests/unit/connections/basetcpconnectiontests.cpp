//
// Created by Tomas Gallucci on 6/14/26.
//

#include "basetcpconnectiontests.h"

#include "incomingtcpthread.h"
#include "../../../connections/basetcpconnection.h"
#include "../utils/testutils.h"

#include <QtTest>

#include "outgoingtcpthread.h"
#include "../testdoubles/basetcpthreadtestdouble.h"
#include "testdoubles/connections/BaseTcpConnectionTestDouble.h"

std::unique_ptr<BaseTcpThreadTestDouble> BaseTcpConnectionTests::createThreadWithConnectionState(
    QAbstractSocket::SocketState socketState)
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(socketState);

    return std::make_unique<BaseTcpThreadTestDouble>(mockSock);
}

// isConnected() tests
void BaseTcpConnectionTests::testIsConnected_data()
{
    QTest::addColumn<QAbstractSocket::SocketState>("socketState");
    QTest::addColumn<bool>("expectedReturnValue");


    QTest::newRow("connected")  << QAbstractSocket::SocketState::ConnectedState << true;
    QTest::newRow("not connected")  << QAbstractSocket::SocketState::UnconnectedState << false;
}

void BaseTcpConnectionTests::testIsConnected()
{
    QFETCH(QAbstractSocket::SocketState, socketState);
    QFETCH(bool, expectedReturnValue);

    auto mockSock = std::make_unique<MockSslSocket>();
    mockSock->setMockState(socketState);

    auto thread = std::make_unique<BaseTcpThreadTestDouble>(mockSock.release());
    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));
    // BaseTcpConnection connection(std::move(thread));

    QCOMPARE(connection.isConnected(), expectedReturnValue);
}

/*
 * I've left the next bit commented out instead of deleting it
 * so I know how to do return this kind of thing later. I'll try to remember to delete it
 * before I submit the PR for final review.
 */

// void BaseTcpConnectionTests::testIsConnected_data()
// {
//     QTest::addColumn<BaseTcpThreadTestDouble*>("thread");
//     QTest::addColumn<bool>("expectedReturnValue");
//
//
//
//     QTest::newRow("connected")  << createThreadWithConnectionState(QAbstractSocket::SocketState::ConnectedState).release() << true;
//     QTest::newRow("not connected")  << createThreadWithConnectionState(QAbstractSocket::SocketState::UnconnectedState).release() << false;
// }

// void BaseTcpConnectionTests::testIsConnected()
// {
//     QFETCH(BaseTcpThreadTestDouble*, thread);
//     QFETCH(bool, expectedReturnValue);
//
//     std::unique_ptr<BaseTcpThreadTestDouble> threadGuard(thread);
//     auto connection = Connection(std::move(threadGuard));
//     QCOMPARE(thread->isConnected(), expectedReturnValue);
// }

// isSecure() tests
void BaseTcpConnectionTests::testIsSecure_data()
{
    QTest::addColumn<bool>("isSecure");
    QTest::addColumn<bool>("expectedReturnValue");

    QTest::newRow("true")  << true << true;
    QTest::newRow("false")  << false << false;
}

void BaseTcpConnectionTests::testIsSecure()
{
    QFETCH(bool, isSecure);
    QFETCH(bool, expectedReturnValue);

    auto mockSock = std::make_unique<MockSslSocket>();
    mockSock->setMockEncrypted(isSecure);

    auto thread = std::make_unique<BaseTcpThreadTestDouble>(mockSock.release());
    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));

    QCOMPARE(connection.isSecure(), expectedReturnValue);
}

// isIncoming() tests
void BaseTcpConnectionTests::testIsIncoming_returnsTrue_whenThreadIsIncomingTcpThread()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketForTest());
    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));
    QCOMPARE(connection.isIncoming(), true);
}

void BaseTcpConnectionTests::testIsIncoming_returnsFalse_whenThreadIsOutgoingTcpThread()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketForTest());
    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));
    QCOMPARE(connection.isIncoming(), true);
}

// isPersistent() tests
// ==================== INCOMING TESTS ====================

void BaseTcpConnectionTests::testIsPersistent_Incoming_data()
{
    QTest::addColumn<bool>("isPersistent");

    QTest::newRow("true")  << true;
    QTest::newRow("false") << false;
}

void BaseTcpConnectionTests::testIsPersistent_Incoming()
{
    QFETCH(bool, isPersistent);

    constexpr bool isEncrypted = false;
    const auto socket = TestUtils::createMockSocketForTest();

    auto thread = std::make_unique<IncomingTcpThread>(socket, isEncrypted, isPersistent);

    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));
    QCOMPARE(connection.isPersistent(), isPersistent);
}

// ==================== OUTGOING TESTS ====================

void BaseTcpConnectionTests::testIsPersistent_Outgoing_data()
{
    QTest::addColumn<bool>("isPersistent");

    QTest::newRow("true")  << true;
    QTest::newRow("false") << false;
}

void BaseTcpConnectionTests::testIsPersistent_Outgoing()
{
    QFETCH(bool, isPersistent);

    const auto socket = TestUtils::createMockSocketForTest();
    auto packet = TestUtils::createPacketForTest();
    packet.persistent = isPersistent;

    auto thread = std::make_unique<OutgoingTcpThread>(socket, packet);

    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));
    QCOMPARE(connection.isPersistent(), isPersistent);
}

void BaseTcpConnectionTests::testGetClassName()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketForTest());
    auto connection = BaseTcpConnectionTestDouble();
    connection.setThread(std::move(thread));
    QCOMPARE(connection.callGetClassName(), "BaseTcpConnectionTestDouble");
}
