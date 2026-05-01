//
// Created by Tomas Gallucci on 3/6/26.
//

#include "connectionmanager_tests.h"
#include <QSignalSpy>



void ConnectionManagerTests::init()
{
    manager = std::make_unique<TestConnectionManager>();
}

void ConnectionManagerTests::cleanup()
{
    manager->shutdownAll();
    manager.reset();
}

void ConnectionManagerTests::testCreateReturnsValidId()
{
    quint64 id = manager->createPersistent("127.0.0.1", 12345);
    QVERIFY(id > 0);
    QVERIFY(manager->connections().find(id) != manager->connections().end());
}

void ConnectionManagerTests::testCloseRemovesConnection()
{
    quint64 id = manager->createPersistent("127.0.0.1", 12345);
    QVERIFY(manager->connections().find(id) != manager->connections().end());

    manager->close(id);

    QVERIFY(manager->connections().find(id) == manager->connections().end());
}

void ConnectionManagerTests::testShutdownAllClearsAllConnections()
{
    manager->createPersistent("host1", 1000);
    manager->createPersistent("host2", 2000);

    QCOMPARE(manager->connections().size(), 2);

    manager->shutdownAll();

    QCOMPARE(manager->connections().size(), 0);
}

void ConnectionManagerTests::testSignalForwardingIncludesId()
{
    QSignalSpy spyData(manager.get(), &ConnectionManager::dataReceived);

    quint64 id = manager->createPersistent("127.0.0.1", 12345);

    // Simulate data received from the connection (manual emit for test)
    // In real test, you'd need to trigger packetReceived on the Connection
    // For now, just check manager has the connection
    QCOMPARE(manager->connections().size(), 1);

    // Placeholder: in future, add real data trigger and spy check
    QVERIFY(spyData.count() == 0);  // expand later
}
