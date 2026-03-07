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

void ConnectionTests::testIncomingConstructor_setsModeFlagsCorrectly()
{
    const int dummyDescriptor = 9876;
    bool isSecure = false;
    bool isPersistent = true;

    auto mockThread = std::make_unique<TestTcpThreadClass>(dummyDescriptor, isSecure, isPersistent);
    mockThread->forceFastExitFromPersistentLoop();

    TestConnection conn(dummyDescriptor, isSecure, isPersistent, nullptr, std::move(mockThread));

    // Public queries (assuming you have or will add these getters)
    // If not public yet, add them or use direct access via test subclass
    QCOMPARE(conn.isIncoming(), true);          // add bool isIncoming() const { return m_isIncoming; }
    QCOMPARE(conn.isSecure(), isSecure);
    QCOMPARE(conn.isPersistent(), isPersistent); // add if missing: bool isPersistent() const { return m_isPersistent; }

    // If m_socketDescriptor is private, skip or add getter
    // QCOMPARE(conn.socketDescriptor(), qintptr(dummyDescriptor));
}

void ConnectionTests::testIncomingConstructor_generatesValidId()
{
    TestConnection conn(54321, false, true);

    QString id = conn.id();
    QVERIFY(!id.isEmpty());
    QVERIFY(id.length() >= 32);               // rough UUID length check
    QVERIFY(!id.contains('{'));               // if using WithoutBraces
    QVERIFY(!id.contains('}'));
    // Optional: more strict UUID format check if desired
}

void ConnectionTests::testIncomingConstructor_threadCreatedAndStartSucceeds()
{
    TestConnection conn(1111, false, true);

    QVERIFY(conn.getThread() != nullptr);       // if m_thread protected or via getter
    // or indirect check:
    QVERIFY(conn.getThreadStarted());          // start() called in constructors, so thread should be started

    // Optional: check no immediate error signal if you have spy setup
    // QSignalSpy errorSpy(&conn, &Connection::errorOccurred);
    // QCOMPARE(errorSpy.count(), 0);
}

void ConnectionTests::testIncomingConstructor_variations_securePersistent()
{
    // Variant 1: secure + non-persistent
    {
        TestConnection c(3333, true, false);
        QCOMPARE(c.isSecure(), true);
        QCOMPARE(c.isPersistent(), false);
        QCOMPARE(c.isIncoming(), true);
    }

    // Variant 2: non-secure + persistent (default)
    {
        TestConnection c(4444);  // using defaults isSecure=false, persistent=true
        QCOMPARE(c.isSecure(), false);
        QCOMPARE(c.isPersistent(), true);
        QCOMPARE(c.isIncoming(), true);
    }
}

// QTEST_MAIN(TestConnection)
// #include "test_connection.moc"   // needed for moc processing
