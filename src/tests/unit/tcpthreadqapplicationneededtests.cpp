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
