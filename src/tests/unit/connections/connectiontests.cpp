//
// Created by Tomas Gallucci on 6/16/26.
//

#include <qtestcase.h>

#include "connectiontests.h"

#include "ConnectionStatusMessages.h"
#include "connections/connection.h"
#include "../testdoubles/connections/ConnectionTestDouble.h"
#include "testdoubles/connections/BaseTcpConnectionTestDouble.h"

void ConnectionTests::testDefaultStateIsCreated()
{
    const ConnectionTestDouble connection{}; // because Connection is a virtual class
    QCOMPARE(connection.getConnectionState(), Connection::State::Created);
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
