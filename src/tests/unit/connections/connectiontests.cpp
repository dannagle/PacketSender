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
    QTRY_COMPARE_EQ_WITH_TIMEOUT(connection.getConnectionState(), Connection::State::Active, 1000);
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
    QTRY_COMPARE_EQ_WITH_TIMEOUT(connection.getConnectionState(), Connection::State::Inactive, 1000);
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
    QTRY_COMPARE_EQ_WITH_TIMEOUT(conn.getConnectionState(), Connection::State::Active, 1000);
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
    QTRY_COMPARE_EQ_WITH_TIMEOUT(conn.getConnectionState(), Connection::State::Inactive, 1000);
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
