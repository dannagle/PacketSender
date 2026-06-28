//
// Created by Tomas Gallucci on 3/6/26.
//

#include "connectionmanager_tests.h"
#include "testdoubles/connections/connectionmanagertestdouble.h"
#include <QSignalSpy>



// void ConnectionManagerTests::init()
// {
//     manager = std::make_unique<TestConnectionManager>();
// }
//
// void ConnectionManagerTests::cleanup()
// {
//     manager->shutdownAll();
//     manager.reset();
// }
//

void ConnectionManagerTests::testCreateIncomingTcpConnection_returnsPair()
{
    ConnectionManagerTestDouble manager;
    QCOMPARE(manager.getMap().size(), 0);

    auto pair = manager.createIncomingTcpConnection();
    QCOMPARE(manager.getMap().size(), 1);
    QCOMPARE(pair.first, 1);
    QCOMPARE(pair.second->metaObject()->className(), "IncomingTcpConnection");
}

void ConnectionManagerTests::testCreateOutgoingTcpConnection_returnsPair()
{
    ConnectionManagerTestDouble manager;
    QCOMPARE(manager.getMap().size(), 0);

    auto pair = manager.createOutgoingTcpConnection();
    QCOMPARE(manager.getMap().size(), 1);
    QCOMPARE(pair.first, 1);
    QCOMPARE(pair.second->metaObject()->className(), "OutgoingTcpConnection");
}

void ConnectionManagerTests::testCreateMultipleConnections_idIncreasesMonotonically()
{
    ConnectionManagerTestDouble manager;
    QCOMPARE(manager.getMap().size(), 0);

    auto pair1 = manager.createOutgoingTcpConnection();
    QCOMPARE(manager.getMap().size(), 1);
    QCOMPARE(pair1.first, 1);

    auto pair2 = manager.createIncomingTcpConnection();
    QCOMPARE(manager.getMap().size(), 2);
    QCOMPARE(pair2.first, 2);
}

void ConnectionManagerTests::testClose()
{
    int expectedSize = 0;

    ConnectionManagerTestDouble manager;
    QCOMPARE(manager.getMap().size(), expectedSize);

    // Test Philosophy: we can demonstrate this works with 2 connections,
    // but two connections gives us a 50/50 chance of screwing up the test
    // setup but still getting the expected result. 3 elements make inference.

    auto pair1 = manager.createOutgoingTcpConnection();
    QCOMPARE(manager.getMap().size(), ++expectedSize);
    QCOMPARE(pair1.first, 1);

    auto pair2 = manager.createIncomingTcpConnection();
    QCOMPARE(manager.getMap().size(), ++expectedSize);
    QCOMPARE(pair2.first, 2);

    auto pair3 = manager.createIncomingTcpConnection();
    QCOMPARE(manager.getMap().size(), ++expectedSize);
    QCOMPARE(pair3.first, 3);

    manager.close(2);
    QCOMPARE(manager.getMap().size(), --expectedSize);

    auto map = std::move(manager.getMap());
    QVERIFY(map.find(1) != map.end());
    QVERIFY(map.find(3) != map.end());
}

// shutdownAll() tests
void ConnectionManagerTests::testShutdownAll()
{
    ConnectionManagerTestDouble manager;
    QCOMPARE(manager.getMap().size(), 0);

    manager.createOutgoingTcpConnection();
    manager.createIncomingTcpConnection();
    QCOMPARE(manager.getMap().size(), 2);

    manager.shutdownAll();
    QCOMPARE(manager.getMap().size(), 0);
}
