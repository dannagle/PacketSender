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

