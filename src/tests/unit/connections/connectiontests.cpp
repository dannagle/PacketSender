//
// Created by Tomas Gallucci on 6/16/26.
//

#include <qtestcase.h>

#include "connectiontests.h"

#include "ConnectionStatusMessages.h"
#include "connections/connection.h"
#include "../testdoubles/connections/ConnectionTestDouble.h"
#include "testdoubles/connections/BaseTcpConnectionTestDouble.h"
#include "testdoubles/connections/incomingtcpconnectiontestdouble.h"
#include "testdoubles/connections/outgoingtcpconnectiontestdouble.h"
#include "utils/testutils.h"

void ConnectionTests::testDefaultStateIsCreated()
{
    const ConnectionTestDouble connection{}; // because Connection is a virtual class
    QCOMPARE(connection.getConnectionState(), Connection::State::Created);
}

void ConnectionTests::testSocketSuccessfullySetsSocketDescriptor_TransitionsStateToActive()
{
    IncomingTcpConnectionTestDouble connection {};
    QCOMPARE(connection.getConnectionState(), Connection::State::Created);

    constexpr bool isSecure = false;
    constexpr bool isPersistent = false;

    QSignalSpy stateChangedSpy(&connection, &Connection::stateChanged);

    connection.receiveData(TestUtils::startQTcpServer().socketDescriptor(), isSecure, isPersistent);
    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(
            stateChangedSpy, ConnectionStatusMessages::INCOMING_CONNECTION_ACCEPTED()
        ),
    1500);
    QTRY_VERIFY_WITH_TIMEOUT(!connection.states.empty(), 1500);

    QDEBUG() << "connection.states.size(): " << connection.states.size();
    QCOMPARE(connection.states[connection.states.size() -2], Connection::State::Active);
}

void ConnectionTests::testSocketDisconnectedBeforeIncomingDataRead_TransitionsStateToInactive()
{
    IncomingTcpConnectionTestDouble connection {};
    QCOMPARE(connection.getConnectionState(), Connection::State::Created);

    constexpr bool isSecure = false;
    constexpr bool isPersistent = false;

    QSignalSpy stateChangedSpy(&connection, &Connection::stateChanged);

    connection.desiredSocketConfigurationForTest =
        IncomingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_INCOMING_PLAIN_TCP_UNSUCCESSFUL_READ;
    connection.receiveData(TestUtils::startQTcpServer().socketDescriptor(), isSecure, isPersistent);
    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(
            stateChangedSpy, ConnectionStatusMessages::ERROR_SOCKET_NOT_CONNECTED()
        ),
    1500);
    QTRY_VERIFY_WITH_TIMEOUT(!connection.states.empty(), 1500);
    QCOMPARE(connection.states.size(), 1);
    QDEBUG() << "ConnectionTests::testSocketDisconnectedBeforeIncomingDataRead_TransitionsStateToInactive states: " << connection.printStates();
    QCOMPARE(connection.states[0], Connection::State::Inactive);
}

void ConnectionTests::testIncomingSSL_success_setsConnectionStateToActive()
{
    IncomingTcpConnectionTestDouble conn;
    conn.desiredSocketConfigurationForTest =
        IncomingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_INCOMING_SSL_SUCCESS;

    QSignalSpy stateChangedSpy(&conn, &Connection::stateChanged);

    constexpr bool isSecure = true;
    constexpr bool isPersistent = false;

    conn.receiveData(TestUtils::startQTcpServer().socketDescriptor(), isSecure, isPersistent);

    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(stateChangedSpy, ConnectionStatusMessages::SSL_CONNECTED()),
        1500);
    QTRY_VERIFY_WITH_TIMEOUT(!conn.states.empty(), 1500);
    QDEBUG() << "ConnectionTests::testIncomingSSL_success_setsConnectionStateToActive states.size(): " << conn.states.size();
    QDEBUG() << "ConnectionTests::testIncomingSSL_success_setsConnectionStateToActive states: " << conn.printStates();
    QCOMPARE(conn.states[conn.states.size() -1], Connection::State::Inactive);
}

void ConnectionTests::testIncomingSSL_failure_setsConnectionStateToInactive()
{
    IncomingTcpConnectionTestDouble conn;
    conn.desiredSocketConfigurationForTest =
        IncomingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_INCOMING_SSL_FAILURE;

    QSignalSpy stateChangedSpy(&conn, &Connection::stateChanged);

    constexpr bool isSecure = true;
    constexpr bool isPersistent = false;

    conn.receiveData(TestUtils::startQTcpServer().socketDescriptor(), isSecure, isPersistent);

    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(stateChangedSpy, ConnectionStatusMessages::SSL_HANDSHAKE_FAILED()),
        1500);
    QTRY_VERIFY_WITH_TIMEOUT(!conn.states.empty(), 1500);
    QCOMPARE(conn.states[conn.states.size() -2], Connection::State::Inactive);
}

void ConnectionTests::testHandleOutgoingSSL_success_setsConnectionStateToActive()
{
    auto conn = OutgoingTcpConnectionTestDouble();
    conn.desiredSocketConfigurationForTest =
        OutgoingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_OUTGOING_SSL_SUCCESS;

    QSignalSpy stateChangedSpy(&conn, &Connection::stateChanged);

    Packet testPacket = TestUtils::createPacketForTest();  // assume you have this helper
    testPacket.tcpOrUdp = "SSL";
    conn.send(testPacket);

    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(stateChangedSpy, ConnectionStatusMessages::SSL_CONNECTED()),
        2000);
    QTRY_VERIFY_WITH_TIMEOUT(!conn.states.empty(), 1500);
    QCOMPARE(conn.states[conn.states.size() -2], Connection::State::Active);
}

void ConnectionTests::testHandleOutgoingSSL_failure_setsConnectionStateToInactive()
{
    auto conn = OutgoingTcpConnectionTestDouble();
    conn.desiredSocketConfigurationForTest =
        OutgoingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_OUTGOING_SSL_UNSUCCESSFUL;

    QSignalSpy stateChangedSpy(&conn, &Connection::stateChanged);

    Packet testPacket = TestUtils::createPacketForTest();  // assume you have this helper
    testPacket.tcpOrUdp = "SSL";
    conn.send(testPacket);

    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(stateChangedSpy, ConnectionStatusMessages::SSL_HANDSHAKE_FAILED()),
        2000);
    QTRY_VERIFY_WITH_TIMEOUT(conn.states.size() == 1, 1500);
    QCOMPARE(conn.states[0], Connection::State::Inactive);
}

void ConnectionTests::testTerminate_TransitionsStateToClosing_whenThreadIsNullptr()
{
    BaseTcpConnectionTestDouble connection {};
    QCOMPARE(connection.getConnectionState(), Connection::State::Created);

    connection.callTerminateConnection();
    QCOMPARE(connection.getConnectionState(), Connection::State::Closing);
}

void ConnectionTests::testTerminate_TransitionsStateToClosed_whenThreadIsNOTNullptr()
{
    BaseTcpConnectionTestDouble connection {};
    QCOMPARE(connection.getConnectionState(), Connection::State::Created);

    auto thread = std::make_unique<BaseTcpThreadTestDouble>(TestUtils::createMockSocketForTest());
    connection.setThread(std::move(thread));

    connection.callTerminateConnection();
    QCOMPARE(connection.getConnectionState(), Connection::State::Closed);
}

void ConnectionTests::testHandleOutgoingPlainTCP_success_setsConnectionStateToActive()
{
    auto conn = OutgoingTcpConnectionTestDouble();
    QDEBUG() << conn.id();
    conn.desiredSocketConfigurationForTest =
        OutgoingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_OUTGOING_PLAIN_TCP_SUCCESS;

    QSignalSpy stateChangedSpy(&conn, &Connection::stateChanged);


    Packet testPacket = TestUtils::createPacketForTest();
    conn.send(testPacket);

    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(
            stateChangedSpy, ConnectionStatusMessages::CONNECTED()
        ),
    1500);
    QTRY_VERIFY_WITH_TIMEOUT(!conn.states.empty(), 1500);
    QCOMPARE(conn.states[conn.states.size() -2], Connection::State::Active);
}

void ConnectionTests::testHandleOutgoingPlainTCP_unsuccessful_setsConnectionStateToInactive()
{
    auto conn = OutgoingTcpConnectionTestDouble();
    QDEBUG() << conn.id();
    conn.desiredSocketConfigurationForTest =
        OutgoingTcpConnectionTestDouble::SocketConfigurationForTest::HANDLE_OUTGOING_PLAIN_TCP_UNSUCCESSFUL;

    QSignalSpy stateChangedSpy(&conn, &Connection::stateChanged);


    Packet testPacket = TestUtils::createPacketForTest();
    conn.send(testPacket);

    QTRY_VERIFY_WITH_TIMEOUT(
        TestUtils::signalSpyContainsMessage(
            stateChangedSpy, ConnectionStatusMessages::COULD_NOT_CONNECT()
        ),
    1500);
    QTRY_VERIFY_WITH_TIMEOUT(!conn.states.empty(), 1500);
    QCOMPARE(conn.states.size(), 1);
    QCOMPARE(conn.states[0], Connection::State::Inactive);
}



void ConnectionTests::testConnectionConstructor_createsConnectionObjectWithID()
{
    const ConnectionTestDouble connection{};

    const QString id = connection.id();
    QUuid uuid(id);
    QVERIFY(uuid.isNull() == false);
    QCOMPARE(uuid.toString(QUuid::WithoutBraces), id);
}

void ConnectionTests::testGetClassName()
{
    const ConnectionTestDouble connection{};
    QCOMPARE(connection.callGetClassName(), "ConnectionTestDouble");
}
