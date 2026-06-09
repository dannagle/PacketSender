//
// Created by Tomas Gallucci on 6/8/26.
//

#include <QtTest>

#include "incomingtcpthreadtests.h"

#include <QTcpServer>

#include "testutils.h"
#include "../../incomingtcpthread.h"
#include "testdoubles/incomingtcpthreadtestdouble.h"

QTcpServer& startQTcpServer()
{
    auto *server = new QTcpServer();
    server->listen(QHostAddress::LocalHost);
    return *server;
}

int getValidDescriptor()
{
    const int validDescriptor = static_cast<int>(startQTcpServer().socketDescriptor());
    QDEBUG() << "validDescriptor: " << validDescriptor;
    return validDescriptor;
}


// constructor tests
void IncomingTcpThreadTests::testConstructor_assignsSocketDescriptor()
{
    const int validDescriptor = getValidDescriptor();

    auto const thread = IncomingTcpThread(validDescriptor);
    QCOMPARE(thread.getSocketDescriptor(), validDescriptor);
}

void IncomingTcpThreadTests::testConstructor_assignsIsSecure()
{
    auto const thread = IncomingTcpThread(getValidDescriptor(), true);
    QCOMPARE(thread.getShouldUseSSL(), true);
}


// buildInitialReceivedPacket() tests
void IncomingTcpThreadTests::testBuildInitialReceivedPacket_socketInterfaceIsNullptr_data()
{
    QTest::addColumn<bool>("isSecure");
    QTest::addColumn<QString>("protocolName");

    QTest::newRow("unencrypted")  << false << "TCP";
    QTest::newRow("encrypted")        << true  << "SSL";
}

void IncomingTcpThreadTests::testBuildInitialReceivedPacket_socketInterfaceIsNullptr()
{
    QFETCH(bool, isSecure);
    QFETCH(QString, protocolName);

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest(), isSecure);
    thread.setSocketForTest(nullptr);

    const auto returnedPacket = thread.callBuildInitialReceivedPacket();
    QVERIFY(returnedPacket.timestamp.isValid());
    QCOMPARE(returnedPacket.name, returnedPacket.timestamp.toString(DATETIMEFORMAT));

    auto normalizedPacket = returnedPacket;
    normalizedPacket.name = "";

    Packet p;
    p.name = "";
    p.tcpOrUdp = protocolName;
    p.fromIP = "";
    p.toIP = "You";
    p.port = 0;
    p.fromPort = 0;
    QCOMPARE(normalizedPacket, p);
}

void IncomingTcpThreadTests::testBuildInitialReceivedPacket_socketInterfaceIsNotNullptr()
{
    const QString mockDataString = "foo bar baz";
    QByteArray mockReadData = mockDataString.toUtf8();

    auto sock = TestUtils::createMockSocketForTest();
    sock->setMockReadData(mockReadData);

    auto thread = IncomingTcpThreadTestDouble(sock);

    const auto returnedPacket = thread.callBuildInitialReceivedPacket();
    QVERIFY(returnedPacket.timestamp.isValid());
    QCOMPARE(returnedPacket.name, returnedPacket.timestamp.toString(DATETIMEFORMAT));

    auto normalizedPacket = returnedPacket;
    normalizedPacket.name = "";

    Packet p;
    p.name = "";
    p.tcpOrUdp = "TCP";
    p.fromIP = "127.0.0.1";
    p.toIP = "You";
    p.port = 0;
    p.fromPort = 0;
    p.hexString = mockReadData.toHex(' ').toUpper() + " ";
    QCOMPARE(normalizedPacket, p);
}
