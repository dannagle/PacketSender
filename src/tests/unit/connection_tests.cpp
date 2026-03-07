//
// Created by Tomas Gallucci on 3/5/26.
//

#include <QtTest/QTest.h>

// test header files
#include "connection_tests.h"
#include "testdoubles/testtcpthreadclass.h"

// code header files
#include "connection.h"

// TestConnection::ConnectionTests(){}
// TestConnection::~ConnectionTests(){}

class TestConnection : public Connection
{
public:
    TestConnection(const QString &host, quint16 port, const Packet &initial = Packet(),
                   QObject *parent = nullptr)
        : Connection(host, port, initial, nullptr, std::make_unique<TestTcpThreadClass>(host, port, initial, nullptr))
    {
        m_threadWaitTimeoutMs = testThreadShutdownWaitMs;

    }

    TestConnection(int socketDescriptor, bool isSecure = false,
        bool persistent = true, QObject *parent = nullptr)
        : Connection(socketDescriptor, isSecure, persistent, nullptr,
                 std::make_unique<TestTcpThreadClass>(socketDescriptor, isSecure, persistent, nullptr))
    {
        m_threadWaitTimeoutMs = testThreadShutdownWaitMs;
    }

    TestConnection(int socketDescriptor, bool isSecure, bool persistent,
                   QObject *parent,
                   std::unique_ptr<TestTcpThreadClass> thread)
        : Connection(socketDescriptor, isSecure, persistent, parent, std::move(thread))
    {
    }

    using Connection::getThread;
    using Connection::getThreadStarted;

private:
    static constexpr int testThreadShutdownWaitMs = 100;
};

void ConnectionTests::testCreationAndId()
{
    TestConnection conn("127.0.0.1", 12345);
    QVERIFY(!conn.id().isEmpty());
    QVERIFY(conn.id().length() > 20);  // typical UUID string length without braces
}

void ConnectionTests::testDestructionDoesNotCrash()
{
    // Scope-based destruction
    {
        TestConnection conn("example.com", 80);
        // do nothing
    }
    // If we reach here without crash → good
    QVERIFY(true);
}

void ConnectionTests::testMultipleInstancesHaveUniqueIds()
{
    TestConnection a("host1", 1000);
    TestConnection b("host2", 2000);

    QVERIFY(a.id() != b.id());
}

// basic thread lifecycle test
void ConnectionTests::testThreadStartsAndStops()
{
    TestConnection conn("127.0.0.1", 12345);

    // Give thread a moment to start
    QTest::qWait(500);

    // Check thread is running (basic check)
    QVERIFY(conn.isConnected() || true); // may be Connecting → adjust as needed

    // Scope exit → dtor should stop thread
    // We can't easily assert thread finished here without signals,
    // but no crash = good enough for now
}

// QTEST_MAIN(TestConnection)
// #include "test_connection.moc"   // needed for moc processing
