//
// Created by Tomas Gallucci on 4/11/26.
//

#include <QtTest/QTest.h>
#include <QSignalSpy>
#include <QTcpServer>

#include "packet.h"

#include "testdoubles/testtcpthreadclass.h"

#include <memory>

#include "testutils.h"

#include "persistentconnectionlooptests.h"

// HELPERS
void PersistentConnectionLoopTests::dumpStatusSpy(const QSignalSpy& statusSpy)
{
    qDebug() << "Status signals received:" << statusSpy.count();
    for (const auto& args : statusSpy) {
        qDebug() << "  Status:" << args.first().toString();
    }
}

// TESTS
void PersistentConnectionLoopTests::testPrepareForPersistentLoop_preparesSendPacketCorrectly()
{
    Packet initialPacket;
    initialPacket.hexString = "AA BB CC DD";
    initialPacket.port = 12345;
    initialPacket.fromIP = "192.168.1.100";
    initialPacket.fromPort = 54321;        // explicitly set for test

    TestTcpThreadClass thread("127.0.0.1", 54321, Packet());

    thread.callPrepareForPersistentLoop(initialPacket);

    Packet sendPacket = thread.getSendPacket();

    QCOMPARE(sendPacket.persistent, true);
    QVERIFY(sendPacket.hexString.isEmpty());
    QCOMPARE(sendPacket.port, 12345);
    QCOMPARE(sendPacket.fromIP, QString("192.168.1.100"));
    QCOMPARE(sendPacket.fromPort, 54321);
}

void PersistentConnectionLoopTests::testPrepareForPersistentLoop_setsUpClientConnection()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    quint16 serverPort = server.serverPort();
    QTest::qWait(100);

    QSslSocket clientSock;
    clientSock.connectToHost("127.0.0.1", serverPort);
    QVERIFY(clientSock.waitForConnected(2000));

    QTest::qWait(200);

    std::unique_ptr<QTcpSocket> acceptedSock(server.nextPendingConnection());
    QVERIFY(acceptedSock);

    TestTcpThreadClass thread("127.0.0.1", serverPort, Packet());

    // Important: set the socket descriptor so prepareForPersistentLoop can use it
    thread.setSocketDescriptor(acceptedSock->socketDescriptor());

    Packet initialPacket;
    initialPacket.port = serverPort;

    thread.callPrepareForPersistentLoop(initialPacket);

    QVERIFY(thread.clientSocket() != nullptr);
    QCOMPARE(thread.clientSocket()->state(), QAbstractSocket::ConnectedState);
}

void PersistentConnectionLoopTests::testPrepareForPersistentLoop_withRealSocket_updatesPorts()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    quint16 serverPort = server.serverPort();
    QTest::qWait(100);

    QSslSocket clientSock;
    clientSock.connectToHost("127.0.0.1", serverPort);
    QVERIFY(clientSock.waitForConnected(2000));

    QTest::qWait(200);

    std::unique_ptr<QTcpSocket> acceptedSock(server.nextPendingConnection());
    QVERIFY(acceptedSock);

    TestTcpThreadClass thread("127.0.0.1", serverPort, Packet());
    thread.setSocketDescriptor(acceptedSock->socketDescriptor());

    Packet initialPacket;
    initialPacket.port = serverPort;
    initialPacket.fromIP = "127.0.0.1";

    thread.callPrepareForPersistentLoop(initialPacket);

    Packet sendPacket = thread.getSendPacket();

    QCOMPARE(sendPacket.persistent, true);
    QVERIFY(sendPacket.hexString.isEmpty());

    // These should be updated from the real socket
    QCOMPARE(sendPacket.fromPort, serverPort); // peer port on server side
    QVERIFY(sendPacket.port > 0);
    QCOMPARE(sendPacket.fromIP, QString("127.0.0.1"));
}

// persistentConnectionLoop characterization tests

void PersistentConnectionLoopTests::testPersistentLoop_exitsOnCloseRequest()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());
    thread.incomingPersistent = true;
    thread.closeRequest = true;   // trigger early exit

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callPersistentConnectionLoop();

    // The early exit path does NOT emit "Disconnected" — that's only in the cleanup at the bottom
    // So we should check that it exited cleanly without crashing
    QVERIFY(!thread.insidePersistent);

    // Optional: check that it did NOT emit "Connected and idle"
    QVERIFY(!statusSpy.contains(QVariantList{"Connected and idle."}));
}

void PersistentConnectionLoopTests::testPersistentLoop_processesNoDataAndExits()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());
    thread.incomingPersistent = true;

    // Setup mock socket in ConnectedState
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    thread.setClientConnection(mockSock);

    // Prevent immediate early exit and control loop iterations
    thread.forceExitAfterOneIteration = true;   // exits after 1-2 iterations

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callPersistentConnectionLoop();

    // Debug what actually happened
    dumpStatusSpy(statusSpy);

    // Assert the exact sequence/behavior we currently see
    QVERIFY(statusSpy.contains(QVariantList{"Waiting to receive"}));
    QVERIFY(statusSpy.contains(QVariantList{"Reading response"}));
    QVERIFY(statusSpy.contains(QVariantList{"Disconnecting"}));
    QVERIFY(statusSpy.contains(QVariantList{"Disconnected"}));

    // Optional: check we did NOT emit the idle message (to keep the tests distinct)
    QVERIFY(!statusSpy.contains(QVariantList{"Connected and idle."}));
}

void PersistentConnectionLoopTests::testPersistentLoop_emitsIdleStatusWhenNoData()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());
    thread.incomingPersistent = true;
    thread.forceExitAfterOneIteration = true;

    Packet initial;
    initial.hexString.clear();
    initial.persistent = true;           // make sure it's set

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockBytesAvailable(0);
    thread.setClientConnection(mockSock);

    // This should set persistent = true and hexString = empty
    thread.callPrepareForPersistentLoop(initial);

    // Extra safety: force it again right before running the loop
    thread.getSendPacketByReference().persistent = true;   // if you have getSendPacket()

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    QVERIFY2(statusSpy.contains(QVariantList{"Connected and idle."}),
             "Expected 'Connected and idle.' status to be emitted in the idle path");
}

void PersistentConnectionLoopTests::testPersistentLoop_exitsImmediatelyOnCloseRequest()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());
    thread.incomingPersistent = true;
    thread.closeRequest = true;        // Trigger immediate early exit

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    // Should exit immediately without going into the main loop
    // Early exit should skip almost everything, including the final "Disconnected"
    QCOMPARE(statusSpy.count(), 0);
    QVERIFY(!thread.insidePersistent);
}

void PersistentConnectionLoopTests::testPersistentLoop_exitsOnConnectionBroken()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());
    thread.incomingPersistent = true;
    thread.forceExitAfterOneIteration = true;

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(false);           // Force not connected
    mockSock->setMockBytesAvailable(10);         // Non-zero so idle is skipped
    thread.setClientConnection(mockSock);

    // Bypass prepareForPersistentLoop to have full control
    thread.getSendPacketByReference().hexString = "AA BB CC";   // not empty
    thread.getSendPacketByReference().persistent = true;

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    QVERIFY2(statusSpy.contains(QVariantList{"Connection broken"}),
             "Expected 'Connection broken.' status when socket is not connected");
}

void PersistentConnectionLoopTests::testPersistentLoop_cleansUpOnExit()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());
    thread.incomingPersistent = true;
    thread.forceExitAfterOneIteration = true;

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockBytesAvailable(0);
    thread.setClientConnection(mockSock);

    Packet initial;
    initial.hexString.clear();
    initial.persistent = true;
    thread.callPrepareForPersistentLoop(initial);

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    // Verify final cleanup behavior
    QVERIFY(statusSpy.contains(QVariantList{"Disconnected"}));
    QVERIFY(thread.clientSocket() == nullptr || !thread.clientSocket()->isOpen());
}

// cleanupAfterPersistentConnectionLoop() unit tests

void PersistentConnectionLoopTests::testCleanupAfterPersistentConnectionLoop_whenClientConnectionIsNull_emitsDisconnected()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    // Ensure clientConnection is nullptr
    thread.setClientConnection(nullptr);

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callCleanupAfterPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    QVERIFY2(statusSpy.contains(QVariantList{"Disconnected"}),
             "Expected 'Disconnected' to be emitted even when clientConnection is null");

    // verify no other signals were emitted
    QCOMPARE(statusSpy.count(), 1);

    // verify thread.clientConnection remains nullptr
    QCOMPARE(thread.getClientConnection(), nullptr);
}

void PersistentConnectionLoopTests::testCleanupAfterPersistentConnectionLoop_whenSocketIsConnected_performsFullCleanup()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ConnectedState);
    thread.setClientConnection(mockSock);

    thread.set_m_managedByConnection(false);

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    qDebug() << "Before cleanup - clientConnection =" << thread.getClientConnection();
    thread.callCleanupAfterPersistentConnectionLoop();
    qDebug() << "After cleanup - clientConnection =" << thread.getClientConnection();

    dumpStatusSpy(statusSpy);
    QVERIFY(statusSpy.contains(QVariantList{"Disconnected"}));

    // Main observable outcomes of cleanup
    QVERIFY(thread.getClientConnection() == nullptr);

    // this should be the same object as getClientConnection,
    // but we do a dynamic cast, so we're just verifying that
    // the mock is null. Belt and suspenders.
    QVERIFY(thread.getMockSocket() == nullptr);
}

void PersistentConnectionLoopTests::testCleanupAfterPersistentConnectionLoop_whenManagedByConnection_doesNotCallDeleteLater()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ConnectedState);
    thread.setClientConnection(mockSock);

    thread.set_m_managedByConnection(true);   // Key: managed by Connection

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callCleanupAfterPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    QVERIFY(statusSpy.contains(QVariantList{"Disconnected"}));

    // Core cleanup outcomes
    QVERIFY(thread.getClientConnection() == nullptr);

    // Most important assertion for this test:
    // When managed by Connection, deleteLater() should NOT be called
    QCOMPARE(thread.deleteLaterCallCount, 0);
}

void PersistentConnectionLoopTests::testCleanupAfterPersistentConnectionLoop_whenNotManagedByConnection_callsDeleteLater()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ConnectedState);
    thread.setClientConnection(mockSock);

    thread.set_m_managedByConnection(false);   // Key: managed by Connection

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callCleanupAfterPersistentConnectionLoop();

    dumpStatusSpy(statusSpy);

    QVERIFY(statusSpy.contains(QVariantList{"Disconnected"}));

    // Core cleanup outcomes
    QVERIFY(thread.getClientConnection() == nullptr);

    // Most important assertion for this test:
    // When managed by Connection, deleteLater() should NOT be called
    QCOMPARE(thread.deleteLaterCallCount, 1);
}

void PersistentConnectionLoopTests::testGetPeerAddressAsString_returnsCorrectIPv4Format()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    // Setup IPv4 behavior
    thread.setMockIPProtocol(QAbstractSocket::IPv4Protocol);

    // We need a socket for getPeerAddressAsString() to work
    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress("192.168.1.100"));
    thread.setClientConnection(mockSock);

    QString result = thread.getPeerAddressAsString();
    QCOMPARE(result, QString("192.168.1.100"));
}

void PersistentConnectionLoopTests::testGetPeerAddressAsString_returnsCorrectIPv6Format()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    // Setup IPv6 behavior
    thread.setMockIPProtocol(QAbstractSocket::IPv6Protocol);

    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress("::1"));
    thread.setClientConnection(mockSock);

    QString result = thread.getPeerAddressAsString();
    QCOMPARE(result, QString("::1"));
}

void PersistentConnectionLoopTests::testSendCurrentPacket_emitsConnectionStatusWhenDataExists()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    Packet testPacket;
    testPacket.hexString = "AA BB CC DD";
    thread.getSendPacketByReference() = testPacket;

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);

    thread.callSendCurrentPacket();
    QVERIFY(statusSpy.contains(QVariantList{"Sending data:" + testPacket.asciiString()}));
}

void PersistentConnectionLoopTests::testSendCurrentPacket_emitsSentPacketWhenDataExists()
{
    const QString hexString = "AA BB CC DD";
    const QString name = "Test Packet";
    const QString toIP = "127.0.0.1";
    constexpr unsigned int port = 12345;

    TestTcpThreadClass thread(toIP, 12345, Packet());

    // Set up a packet with known data
    Packet testPacket;
    testPacket.hexString = hexString;
    testPacket.name = name;
    testPacket.toIP = toIP;
    testPacket.port = port;
    thread.getSendPacketByReference() = testPacket;

    QSignalSpy packetSentSpy(&thread, &TCPThread::packetSent);
    thread.callSendCurrentPacket();

    // Check that packetSent was emitted
    QCOMPARE(packetSentSpy.count(), 1);

    // Check the actual packet that was emitted
    Packet emittedPacket = packetSentSpy.first().first().value<Packet>();
    QCOMPARE(emittedPacket.hexString, hexString);
    QCOMPARE(emittedPacket.toIP, toIP);
    QCOMPARE(emittedPacket.port, port);
}

void PersistentConnectionLoopTests::testSendCurrentPacket_doesNothingWhenNoDataToSend()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    // Ensure there's no data to send
    Packet emptyPacket;
    emptyPacket.hexString.clear();
    thread.getSendPacketByReference() = emptyPacket;

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);
    QSignalSpy packetSentSpy(&thread, &TCPThread::packetSent);

    thread.callSendCurrentPacket();

    // Should not emit anything
    QCOMPARE(statusSpy.count(), 0);
    QCOMPARE(packetSentSpy.count(), 0);
}

void PersistentConnectionLoopTests::testHandleReceiveBeforeSend_whenDataReceived_processesAndSendsResponse()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockBytesAvailable(10);           // doesn't have to match the length of the data
    mockSock->setMockReadData("AA BB CC DD");
    thread.setClientConnection(mockSock);

    Packet initial;
    initial.receiveBeforeSend = true;
    thread.getSendPacketByReference() = initial;

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);
    QSignalSpy packetSentSpy(&thread, &TCPThread::packetSent);

    thread.callHandleReceiveBeforeSend();

    dumpStatusSpy(statusSpy);

    QVERIFY(statusSpy.contains(QVariantList{"Waiting for data"}));
    QVERIFY(packetSentSpy.count() > 0);
}

void PersistentConnectionLoopTests::testHandleReceiveBeforeSend_whenNoDataReceived_doesNothing()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockBytesAvailable(0);            // no data
    mockSock->setMockReadData(QByteArray());
    thread.setClientConnection(mockSock);

    Packet initial;
    initial.receiveBeforeSend = true;
    thread.getSendPacketByReference() = initial;

    QSignalSpy statusSpy(&thread, &TCPThread::connectStatus);
    QSignalSpy packetSentSpy(&thread, &TCPThread::packetSent);

    thread.callHandleReceiveBeforeSend();

    dumpStatusSpy(statusSpy);

    // When no data is received in receiveBeforeSend mode:
    // - We still emit "Waiting for data"
    // - We emit the debug message "No pre-emptive receive data" (NOT currently asserted
    //   since we'd have to capture the output of the QDEBUG() macro)
    // - We do NOT emit any packetSent signal
    QVERIFY(statusSpy.contains(QVariantList{"Waiting for data"}));
    QCOMPARE(packetSentSpy.count(), 0);
}

void PersistentConnectionLoopTests::testHandleReceiveBeforeSend_setsCorrectPacketFields()
{
    TestTcpThreadClass thread("127.0.0.1", 12345, Packet());

    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockBytesAvailable(10);
    mockSock->setMockReadData(QByteArray::fromHex("AA BB CC DD"));
    thread.setClientConnection(mockSock);

    Packet initial;
    initial.receiveBeforeSend = true;
    thread.getSendPacketByReference() = initial;

    QSignalSpy packetSentSpy(&thread, &TCPThread::packetSent);

    thread.callHandleReceiveBeforeSend();
    QCOMPARE(packetSentSpy.count(), 1);

    Packet received = packetSentSpy.first().first().value<Packet>();
    QVERIFY(!received.hexString.isEmpty());
    QVERIFY(received.timestamp.isValid());
    QCOMPARE(received.toIP, QString("You"));
    QCOMPARE(received.fromIP, thread.getPeerAddressAsString());   // or mock it
}

