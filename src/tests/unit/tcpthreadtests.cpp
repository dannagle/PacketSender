//
// Created by Tomas Gallucci on 3/6/26.
//

#include <QtTest/QTest.h>
#include "tcpthreadtests.h"

#include <QSignalSpy>
#include <QSslServer>
#include <QSslSocket>

#include "testdoubles/testtcpthreadclass.h"

#include "packet.h"
#include "testdoubles/MockSslSocket.h"

#ifndef BINDSKIPPINGTHREAD_H
#define BINDSKIPPINGTHREAD_H

#include "tcpthread.h"

class BindSkippingThread : public TCPThread
{
public:
    BindSkippingThread(const QString &host, quint16 port, const Packet &initialPacket = Packet(), QObject *parent = nullptr)
        : TCPThread(host, port, initialPacket, parent)
    {
    }

protected:
    bool bindClientSocket() override
    {
        qDebug() << "BindSkippingThread: Skipping bind() for test";
        return false;  // skip random bind, let OS assign source port
    }
};

#endif

void TcpThreadTests::testIncomingConstructorBasic()
{
    // Use invalid descriptor (real ones are positive; -1 is common sentinel)
    const TestTcpThreadClass thread(-1, /*isSecure*/ false, /*isPersistent*/ true);

    QVERIFY(thread.getClientConnection() != nullptr);
    QVERIFY(qobject_cast<QSslSocket*>(thread.getClientConnection()) != nullptr);

    QCOMPARE(thread.getSocketDescriptor(), -1);
    QCOMPARE(thread.isSecure, false);
    QCOMPARE(thread.incomingPersistent, true);
    QCOMPARE(thread.getHost(), QString());
    QCOMPARE(thread.getPort(), quint16(0));

    // Optional: check other defaults
    QCOMPARE(thread.sendFlag, false);
    QCOMPARE(thread.consoleMode, false);
    QCOMPARE(thread.getManagedByConnection(), true);
}

void TcpThreadTests::testIncomingConstructorWithSecureFlag()
{
    const TestTcpThreadClass thread(12345, /*isSecure*/ true, /*isPersistent*/ false);

    QVERIFY(thread.getClientConnection() != nullptr);
    QVERIFY(qobject_cast<QSslSocket*>(thread.getClientConnection()) != nullptr);

    QCOMPARE(thread.getSocketDescriptor(), 12345);
    QCOMPARE(thread.isSecure, true);
    QCOMPARE(thread.incomingPersistent, false);
    QCOMPARE(thread.getHost(), QString());
    QCOMPARE(thread.getPort(), quint16(0));
}

//////////////////////////////////////////////////////////////////////
///                 DETERMINE IP MODE TESTS                 /////////
////////////////////////////////////////////////////////////////////

// Test 1: Both host and sendPacket.toIP are IPv4 → returns IPv4Protocol
void TcpThreadTests::testGetIPConnectionProtocol_bothIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("127.0.0.1", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 2: Both are IPv6 → returns IPv6Protocol
void TcpThreadTests::testGetIPConnectionProtocol_bothIPv6_returnsIPv6()
{
    TestTcpThreadClass thread("::1", 80, Packet());
    thread.setSendPacketToIp("::1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);
}

// Test 3: Host IPv4, sendPacket IPv6 → returns IPv6Protocol (prefers sendPacket.toIP)
void TcpThreadTests::testGetIPConnectionProtocol_hostIPv4_packetIPv6_returnsPacketValue()
{
    TestTcpThreadClass thread("127.0.0.1", 80, Packet());
    thread.setSendPacketToIp("::1");  // mismatch
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);  // prefers sendPacket.toIP
}

// Test 4: Host IPv6, sendPacket IPv4 → returns IPv4Protocol (prefers sendPacket.toIP)
void TcpThreadTests::testGetIPConnectionProtocol_hostIPv6_packetIPv4_returnsPacketValue()
{
    TestTcpThreadClass thread("::1", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");  // mismatch
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);  // prefers sendPacket.toIP
}

// Test 5: Host IPv4-mapped IPv6, sendPacket IPv4 → returns IPv6Protocol
void TcpThreadTests::testGetIPConnectionProtocol_hostMappedIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("::ffff:127.0.0.1", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 6: Host empty, sendPacket IPv4 → returns IPv4Protocol
void TcpThreadTests::testGetIPConnectionProtocol_hostEmpty_packetIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 7: Host empty, sendPacket IPv6 → returns IPv6Protocol
void TcpThreadTests::testGetIPConnectionProtocol_hostEmpty_packetIPv6_returnsIPv6()
{
    TestTcpThreadClass thread("", 80, Packet());
    thread.setSendPacketToIp("::1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);
}

// Test 8: Both empty → returns IPv4Protocol (default/fallback)
void TcpThreadTests::testGetIPConnectionProtocol_bothEmpty_returnsIPv4()
{
    TestTcpThreadClass thread("", 80, Packet());
    thread.setSendPacketToIp("");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

// Test 9: Host invalid string, sendPacket IPv4 → returns IPv4Protocol (fallback)
void TcpThreadTests::testGetIPConnectionProtocol_hostInvalid_packetIPv4_returnsIPv4()
{
    TestTcpThreadClass thread("invalid-host-string", 80, Packet());
    thread.setSendPacketToIp("127.0.0.1");
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

//////////////////////////////////////////////////////////////////////
///             tryConnectEncrypted() TESTS                 /////////
////////////////////////////////////////////////////////////////////

void TcpThreadTests::testTryConnectEncrypted_success_emitsCipherAndCertInfo()
{
    MockSslSocket *mockSock = new MockSslSocket();
    TestTcpThreadClass thread(mockSock, "127.0.0.1", 443, Packet());

    qDebug() << "After setter: clientConnection is" << thread.getClientConnection()
        << "and mock is: " << mockSock;

    qDebug() << "Mock connected prior to set:" << mockSock->waitForConnected(0);
    qDebug() << "Mock encrypted prior to set:" << mockSock->waitForEncrypted(0);
    qDebug() << "Mock isEncrypted prior to set (should take default):" << mockSock->isEncrypted();

    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);
    mockSock->setMockSslErrors({});  // no errors
    mockSock->setMockCipher(QSslCipher("TLS_AES_256_GCM_SHA384"));  // or any valid name

    qDebug() << "Mock connected after set:" << mockSock->waitForConnected(0);
    qDebug() << "Mock encrypted after set:" << mockSock->waitForEncrypted(0);
    qDebug() << "Mock isEncrypted should have changed even though we didn't directly assign a boolean:" << mockSock->isEncrypted();

    QSignalSpy packetSpy(&thread, &TCPThread::packetSent);

    bool success = thread.fireTryConnectEncrypted();

    qDebug() << "[SPY] Total packets captured:" << packetSpy.count();

    QStringList messages;
    for (const QVariantList &args : packetSpy) {
        Packet p = args[0].value<Packet>();
        messages << p.errorString;
        qDebug() << "[SPY] Captured message:" << p.errorString;
    }

    QVERIFY(success);
    QCOMPARE(packetSpy.count(), 4);  // cipher, auth, peer cert, our cert

    QVERIFY(messages.contains(QRegularExpression("^Encrypted with.*")));
    QVERIFY(messages.contains(QRegularExpression("^Authenticated with.*")));
    QVERIFY(messages.contains(QRegularExpression("^Peer Cert issued by.*")));
    QVERIFY(messages.contains(QRegularExpression("^Our Cert issued by.*")));
}

void TcpThreadTests::testTryConnectEncrypted_sslErrors_emitsErrorPackets()
{
    MockSslSocket *mockSock = new MockSslSocket();
    TestTcpThreadClass thread(mockSock, "127.0.0.1", 443, Packet());
    thread.setClientConnection(mockSock);

    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(false);

    QList<QSslError> errors = { QSslError(QSslError::SelfSignedCertificate) };
    mockSock->setMockSslErrors(errors);

    QSignalSpy packetSpy(&thread, &TCPThread::packetSent);

    bool success = thread.fireTryConnectEncrypted();

    QVERIFY(!success);
    QVERIFY(packetSpy.count() >= 1);  // error packets + "Not Encrypted!"
}

void TcpThreadTests::testTryConnectEncrypted_connectFailure_returnsFalse()
{
    TestTcpThreadClass thread("127.0.0.1", 443, Packet());
    MockSslSocket *mockSock = new MockSslSocket(&thread);
    thread.setClientConnection(mockSock);

    mockSock->setMockConnected(false);
    mockSock->setMockEncrypted(false);

    bool success = thread.fireTryConnectEncrypted();

    QVERIFY(!success);
}

//////////////////////////////////////////////////////////////////////
///                    clientSocket() TESTS                 /////////
////////////////////////////////////////////////////////////////////
void TcpThreadTests::testClientSocket_lazyCreation_createsRealSocketOnFirstCall()
{
    TestTcpThreadClass thread(nullptr, "127.0.0.1", 80, Packet());

    QSslSocket *sock = thread.clientSocket();
    QVERIFY(sock != nullptr);
    QVERIFY(sock == thread.getClientConnection());
    // QVERIFY(sock->parent() == &thread);
}

void TcpThreadTests::testClientSocket_lazyCreation_returnsExistingSocketOnSecondCall()
{
    TestTcpThreadClass thread("127.0.0.1", 80, Packet());

    QSslSocket *first = thread.clientSocket();
    QVERIFY(first != nullptr);

    QSslSocket *second = thread.clientSocket();
    QCOMPARE(second, first);  // same instance
}

void TcpThreadTests::testInjectionConstructor_assignsMockSocket()
{
    MockSslSocket *mockSock = new MockSslSocket();
    TestTcpThreadClass thread(mockSock, "127.0.0.1", 443, Packet());

    QVERIFY(thread.getClientConnection() == mockSock);
    QVERIFY(mockSock->parent() == &thread);
    // Optional: verify wireupSocketSignals was called (spy on signals or add test hook)
}

void TcpThreadTests::characterizeRun_outgoingSsl_connectsAndEmitsCipherPackets()
{
    // Fixed port for client to target
    const quint16 fixedPort = 8443;

    Packet initialPacket;
    initialPacket.tcpOrUdp = "SSL";
    initialPacket.toIP = "127.0.0.1";
    initialPacket.port = fixedPort;

    BindSkippingThread thread("127.0.0.1", fixedPort, initialPacket, nullptr);
    thread.sendFlag = true;

    QSignalSpy packetSpy(&thread, &TCPThread::packetSent);
    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    // Run directly — no start()
    thread.run();

    // Log what happened
    qDebug() << "Packet count:" << packetSpy.count();
    QStringList messages;
    for (const QVariantList &args : packetSpy) {
        Packet p = args[0].value<Packet>();
        messages << p.errorString;
        qDebug() << "Captured packet:" << p.errorString;
    }

    qDebug() << "Status messages:" << statusSpy;
    for (const QVariantList &args : statusSpy) {
        qDebug() << "Status:" << args[0].toString();
    }

    // Characterization assertions — document current behavior
    QVERIFY(packetSpy.count() >= 1);  // at least one packet (likely failure)
    QVERIFY(messages.contains("Not Encrypted!") || messages.contains("Could not connect"));
}
