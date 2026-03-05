//
// Created by Tomas Gallucci on 3/5/26.
//

#include <QtTest/QTest.h>

// test header files
#include "test_connection.h"

// code header files
#include "connection.h"

// TestConnection::TestConnection(){}
// TestConnection::~TestConnection(){}

void TestConnection::testCreationAndId()
{
    Connection conn("127.0.0.1", 12345);
    QVERIFY(!conn.id().isEmpty());
    QVERIFY(conn.id().length() > 20);  // typical UUID string length without braces
}

void TestConnection::testDestructionDoesNotCrash()
{
    // Scope-based destruction
    {
        Connection conn("example.com", 80);
        // do nothing
    }
    // If we reach here without crash → good
    QVERIFY(true);
}

void TestConnection::testMultipleInstancesHaveUniqueIds()
{
    Connection a("host1", 1000);
    Connection b("host2", 2000);

    QVERIFY(a.id() != b.id());
}

// QTEST_MAIN(TestConnection)
// #include "test_connection.moc"   // needed for moc processing
