//
// Created by Tomas Gallucci on 6/14/26.
//

#include "connectiontests.h"

#include "incomingtcpthread.h"
#include "../../connection.h"
#include "utils/testutils.h"

#include <QUuid>
#include <QtTest>

#include "testdoubles/basetcpthreadtestdouble.h"

// constructor tests
void ConnectionTests::testConnectionConstructor_createsConnectionObjectWithID()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketForTest());
    auto connection = std::make_unique<Connection>(std::move(thread));

    const QString id = connection->id();
    QUuid uuid(id);
    QVERIFY(uuid.isNull() == false);
    QCOMPARE(uuid.toString(QUuid::WithoutBraces), id);
}

std::unique_ptr<BaseTcpThreadTestDouble> ConnectionTests::createThreadWithConnectionState(
    QAbstractSocket::SocketState socketState)
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(socketState);

    return std::make_unique<BaseTcpThreadTestDouble>(mockSock);
}

// isConnected() tests
void ConnectionTests::testIsConnected_data()
{
    QTest::addColumn<QAbstractSocket::SocketState>("socketState");
    QTest::addColumn<bool>("expectedReturnValue");



    QTest::newRow("connected")  << QAbstractSocket::SocketState::ConnectedState << true;
    QTest::newRow("not connected")  << QAbstractSocket::SocketState::UnconnectedState << false;
}

// void ConnectionTests::testIsConnected_data()
// {
//     QTest::addColumn<BaseTcpThreadTestDouble*>("thread");
//     QTest::addColumn<bool>("expectedReturnValue");
//
//
//
//     QTest::newRow("connected")  << createThreadWithConnectionState(QAbstractSocket::SocketState::ConnectedState).release() << true;
//     QTest::newRow("not connected")  << createThreadWithConnectionState(QAbstractSocket::SocketState::UnconnectedState).release() << false;
// }

// void ConnectionTests::testIsConnected()
// {
//     QFETCH(BaseTcpThreadTestDouble*, thread);
//     QFETCH(bool, expectedReturnValue);
//
//     std::unique_ptr<BaseTcpThreadTestDouble> threadGuard(thread);
//     auto connection = Connection(std::move(threadGuard));
//     QCOMPARE(thread->isConnected(), expectedReturnValue);
// }

void ConnectionTests::testIsConnected()
{
    QFETCH(QAbstractSocket::SocketState, socketState);
    QFETCH(bool, expectedReturnValue);

    auto mockSock = std::make_unique<MockSslSocket>();
    mockSock->setMockState(socketState);

    auto thread = std::make_unique<BaseTcpThreadTestDouble>(mockSock.release());
    Connection connection(std::move(thread));

    QCOMPARE(connection.isConnected(), expectedReturnValue);
}

