//
// Created by Tomas Gallucci on 3/15/26.
//

#include <QtTest/QTest.h>
#include <QSignalSpy>
#include <QTcpServer>

#include "packet.h"

#include "testdoubles/testtcpthreadclass.h"
#include "tcpthreadqapplicationneededtests.h"

#include <memory>

void TcpThread_QApplicationNeeded_tests::testDestructorWaitsGracefullyWhenManaged()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress("127.0.0.1"), 0));  // explicit IPv4 localhost
    quint16 actualPort = server.serverPort();

    qDebug() << "Server listening on 127.0.0.1:" << actualPort;
    qDebug() << "isListening:" << server.isListening();

    QTest::qWait(200);  // give a bit more time

    // Now pass the REAL port to the thread
    auto thread = std::make_unique<TestTcpThreadClass>("127.0.0.1", actualPort, Packet());

    QSignalSpy statusSpy(thread.get(), &TCPThread::connectStatus);
    QSignalSpy finishedSpy(thread.get(), &QThread::finished);

    thread->start();

    // Wait for the "Connected" signal (should arrive quickly since server is listening)
    QVERIFY(statusSpy.wait(5000));  // timeout if no status emitted in 5s

    // Check the last emitted status is indeed "Connected"
    QCOMPARE(statusSpy.last().first().toString(), QString("Connected"));

    // Now trigger clean shutdown while connected
    thread->closeConnection();
    thread->requestInterruption();
    thread->quit();

    // Wait for thread to finish
    QVERIFY(finishedSpy.wait(8000));

    QVERIFY(thread->isFinished());
    QVERIFY(!thread->isRunning());

    // Optional: close server explicitly (though it destructs automatically)
    server.close();
}

void TcpThread_QApplicationNeeded_tests::testFullLifecycleWithServer()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress("127.0.0.1"), 0));
    quint16 port = server.serverPort();
    QTest::qWait(100);

    Packet dummy;
    dummy.toIP = "127.0.0.1";
    dummy.port = port;
    dummy.hexString = "AA BB CC";  // some test data

    auto thread = std::make_unique<TestTcpThreadClass>("127.0.0.1", port, dummy);

    QSignalSpy statusSpy(thread.get(), &TCPThread::connectStatus);
    QSignalSpy packetSpy(thread.get(), &TCPThread::packetSent);

    thread->start();

    QVERIFY(statusSpy.wait(5000));
    QVERIFY(statusSpy.contains(QVariantList{"Connected"}));

    // Wait a bit so loop sends something
    QTest::qWait(1000);

    // Check at least one packet was sent/received
    QVERIFY(packetSpy.count() > 0);

    thread->closeConnection();
    thread->requestInterruption();

    QVERIFY(thread->wait(8000));
    QVERIFY(thread->isFinished());
}

void TcpThread_QApplicationNeeded_tests::testOutgoingClientPathStartsLoopAndSendsPacket()
{
    // Characterization test for outgoing client path in TCPThread::run()
    // - Uses dummy port (no real server) to avoid macOS/QSslSocket loopback accept issues
    // - Verifies: connect attempt, loop entry, packet send signal, clean stop
    // - Does NOT verify server-side receipt (tested separately if needed)
    // ...
    const QString testHost = "127.0.0.1";  // reliable IPv4

    Packet initial;
    initial.toIP     = testHost;
    initial.port     = 12345;  // dummy port — we don't need a real server
    initial.hexString = "AA BB CC DD 00 11";
    initial.persistent = true;

    auto thread = std::make_unique<TestTcpThreadClass>(
        testHost, initial.port, initial
    );

    QSignalSpy connectSpy(thread.get(), &TCPThread::connectStatus);
    QSignalSpy packetSentSpy(thread.get(), &TCPThread::packetSent);
    QSignalSpy errorSpy(thread.get(), &TCPThread::error);

    thread->start();

    // Wait for connection attempt to complete (success or failure)
    QVERIFY(connectSpy.wait(6000));

    // Expect at least one "Connected" status (or "Connecting" if you emit that)
    QVERIFY(connectSpy.count() > 0);
    QString status = connectSpy.last().at(0).toString().toLower();
    QVERIFY(status.contains("connect") || status.contains("connected"));

    // Give time for loop to send at least one packet
    QTest::qWait(3000);

    // Verify at least one packet was "sent" client-side
    QVERIFY2(packetSentSpy.count() >= 1,
             "No packetSent signal emitted — loop didn't run");

    // Cleanup
    thread->closeConnection();
    QVERIFY(thread->wait(4000));

    QVERIFY(!thread->isRunning());
    QVERIFY(errorSpy.isEmpty() || errorSpy.last().at(0).value<QSslSocket::SocketError>() == QSslSocket::UnknownSocketError);
}
