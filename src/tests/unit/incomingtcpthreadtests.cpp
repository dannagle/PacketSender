//
// Created by Tomas Gallucci on 6/8/26.
//

#include <QtTest>
#include <QTcpServer>
#include <QSignalSpy>

#include "incomingtcpthreadtests.h"


#include "settingnames.h"
#include "testutils.h"
#include "../../incomingtcpthread.h"
#include "testdoubles/incomingtcpthreadtestdouble.h"
#include "basetcpthread.h"

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

void IncomingTcpThreadTests::testBuildInitialReceivedPacket_socketInterfaceIsInvalid()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->setIsValid(false);

    auto thread = IncomingTcpThreadTestDouble(sock);

    const auto returnedPacket = thread.callBuildInitialReceivedPacket();
    QVERIFY(returnedPacket.hexString.isEmpty());
}

void IncomingTcpThreadTests::testBuildInitialReceivedPacket_socketInterfaceState_isNotConnected()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->setMockState(QAbstractSocket::SocketState::UnconnectedState);

    auto thread = IncomingTcpThreadTestDouble(sock);

    const auto returnedPacket = thread.callBuildInitialReceivedPacket();
    QVERIFY(returnedPacket.hexString.isEmpty());
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

// sendSmartReplyIfConfigured() tests
void IncomingTcpThreadTests::testSendSmartReplyIfConfigured_SendResponseSetting_isFalse()
{
    QSettings& settings = getSettings();
    settings.setValue(SEND_RESPONSE, false);
    settings.sync();

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    auto p = TestUtils::createPacketForTest();
    thread.callSendSmartReplyIfConfigured(p);
    QVERIFY(!thread.wasMethodCalled("sendOutgoingPacket"));
    QCOMPARE(packetSentSpy.count(), 0);
}

void IncomingTcpThreadTests::testSendSmartReplyIfConfigured_SendResponseSetting_defaultValue_isFalse()
{
    QSettings& settings = getSettings();
    settings.setValue(RESPONSE_HEX, "response that will never be sent");
    settings.sync();

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    auto p = TestUtils::createPacketForTest();
    thread.callSendSmartReplyIfConfigured(p);
    QVERIFY(!thread.wasMethodCalled("sendOutgoingPacket"));
    QCOMPARE(packetSentSpy.count(), 0);
}

void IncomingTcpThreadTests::testSendSmartReplyIfConfigured_ResponseHexSetting_isEmptyString()
{
    QSettings& settings = getSettings();
    settings.setValue(SEND_RESPONSE, true);
    settings.setValue(RESPONSE_HEX, "");
    settings.sync();

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    auto p = TestUtils::createPacketForTest();
    thread.callSendSmartReplyIfConfigured(p);
    QVERIFY(!thread.wasMethodCalled("sendOutgoingPacket"));
    QCOMPARE(packetSentSpy.count(), 0);
}

    void IncomingTcpThreadTests::testSendSmartReplyIfConfigured_ResponseHexSetting_defaultValue_isEmptyString()
{
    QSettings& settings = getSettings();
    settings.setValue(SEND_RESPONSE, true);
    settings.sync();

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    auto p = TestUtils::createPacketForTest();
    thread.callSendSmartReplyIfConfigured(p);
    QVERIFY(!thread.wasMethodCalled("sendOutgoingPacket"));
    QCOMPARE(packetSentSpy.count(), 0);
}

void IncomingTcpThreadTests::testSendSmartReplyIfConfigured_successPath()
{
    QSettings& settings = getSettings();
    settings.setValue(SEND_RESPONSE, true);
    settings.setValue(RESPONSE_HEX, "0F BA BA");
    settings.sync();

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    auto p = TestUtils::createPacketForTest();
    thread.callSendSmartReplyIfConfigured(p);
    QVERIFY(thread.wasMethodCalled("sendOutgoingPacket"));
    QCOMPARE(packetSentSpy.count(), 1);

    QCOMPARE(packetSentSpy.first().first().value<Packet>().hexString, "0F BA BA ");
}

void IncomingTcpThreadTests::testSendSmartReplyIfConfigured_successPath_withMacroExpansion()
{
    QSettings& settings = getSettings();
    settings.setValue(SEND_RESPONSE, true);
    settings.setValue(RESPONSE_HEX, "7B 7B 43 4F 55 4E 54 45 52 7D 7D"); // hex for {COUNTER}
    settings.sync();

    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    auto p = TestUtils::createPacketForTest();
    thread.callSendSmartReplyIfConfigured(p);
    QVERIFY(thread.wasMethodCalled("sendOutgoingPacket"));
    QCOMPARE(packetSentSpy.count(), 1);

    QCOMPARE(packetSentSpy.first().first().value<Packet>().hexString.trimmed(), "31");
}
