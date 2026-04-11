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
