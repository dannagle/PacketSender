//
// Created by Tomas Gallucci on 6/8/26.
//

#include <QtTest>
#include <QTcpServer>
#include <QSignalSpy>

#include "incomingtcpthreadtests.h"


#include "settingnames.h"
#include "utils/testutils.h"
#include "../../incomingtcpthread.h"
#include "testdoubles/incomingtcpthreadtestdouble.h"
#include "basetcpthread.h"
#include "settings.h"
#include "utils/calltracker.h"

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

void IncomingTcpThreadTests::init()
{
    QSettings& settings = getSettings();
    settings.clear();
    settings.sync();
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

void IncomingTcpThreadTests::testConstructor_assignsPersistent()
{
    constexpr bool isSecure = true;
    constexpr bool isPersistent = true;
    auto const thread = IncomingTcpThread(getValidDescriptor(), isSecure, isPersistent);
    QCOMPARE(thread.isPersistent(), isPersistent);
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

// emitSSLDiagnosticPackets() tests
void IncomingTcpThreadTests::testEmitSSLDiagnosticPackets_socketNullptr_emits0SentPackets()
{
    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    thread.setSocketForTest(nullptr);

    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);
    thread.callEmitSSLDiagnosticPackets();
    QCOMPARE(packetSentSpy.count(), 0);
}

void IncomingTcpThreadTests::testEmitSSLDiagnosticPackets_socketNotEncrypted_emits0SentPackets()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->setMockEncrypted(false);

    auto thread = IncomingTcpThreadTestDouble(sock);

    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);
    thread.callEmitSSLDiagnosticPackets();
    QCOMPARE(packetSentSpy.count(), 0);
}

Packet normalizeDiagnosticSslPacket(const Packet &packet)
{
    Packet normalizedPacket = packet;

    normalizedPacket.timestamp = QDateTime();
    normalizedPacket.name = "";
    normalizedPacket.errorString = "";

    return normalizedPacket;
}

void IncomingTcpThreadTests::testEmitSSLDiagnosticPackets_successPath()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->setMockEncrypted(true);

    auto thread = IncomingTcpThreadTestDouble(sock);

    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);
    thread.callEmitSSLDiagnosticPackets();
    QCOMPARE(packetSentSpy.count(), 4);

    Packet expectedPacket;
    expectedPacket.toIP = "You";
    expectedPacket.port = 0;
    expectedPacket.fromIP = "127.0.0.1";
    expectedPacket.fromPort = 0;
    expectedPacket.tcpOrUdp = "SSL";

    // === Packet 1: Encryption method ===
    {
        const Packet p = packetSentSpy.at(0).at(0).value<Packet>();

        QVERIFY(p.timestamp.isValid());
        QCOMPARE(p.name, p.timestamp.toString(DATETIMEFORMAT));
        QVERIFY(p.errorString.startsWith("Encrypted with "));

        QCOMPARE(normalizeDiagnosticSslPacket(p), expectedPacket);
    }

    // === Packet 2: Authentication method ===
    {
        const Packet p = packetSentSpy.at(1).at(0).value<Packet>();

        QVERIFY(p.timestamp.isValid());
        QCOMPARE(p.name, p.timestamp.toString(DATETIMEFORMAT));
        QVERIFY(p.errorString.startsWith("Authenticated with "));

        QCOMPARE(normalizeDiagnosticSslPacket(p), expectedPacket);
    }

    // === Packet 3: Peer certificate ===
    {
        const Packet p = packetSentSpy.at(2).at(0).value<Packet>();

        QVERIFY(p.timestamp.isValid());
        QCOMPARE(p.name, p.timestamp.toString(DATETIMEFORMAT));
        QVERIFY(p.errorString.startsWith("Peer cert issued by "));

        QCOMPARE(normalizeDiagnosticSslPacket(p), expectedPacket);
    }

    // === Packet 4: Local certificate ===
    {
        const Packet p = packetSentSpy.at(3).at(0).value<Packet>();

        QVERIFY(p.timestamp.isValid());
        QCOMPARE(p.name, p.timestamp.toString(DATETIMEFORMAT));
        QVERIFY(p.errorString.startsWith("Our Cert issued by "));

        QCOMPARE(normalizeDiagnosticSslPacket(p), expectedPacket);
    }
}
// performSSLHandshakeIfNeeded() tests
void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_shouldUseSslIsFalse_doesNotCallSSLMethods()
{
    constexpr bool shouldUseSSL = false;
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(sock->getCallCount(CallTracker::LOAD_SNAKEOIL_CERTS_()), 0);
    QCOMPARE(sock->getCallCount(CallTracker::LOAD_SSL_CERTS()), 0);
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_socketInterfaceIsNullptr_emitsErrorMessage()
{
    constexpr bool shouldUseSSL = true;
    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest(), shouldUseSSL);
    thread.setSocketForTest(nullptr);

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(errorMessageSpy.count(), 1);
    QCOMPARE(errorMessageSpy.first().first().value<QString>(), "performSSLHandshakeIfNeeded: null socket");

    QCOMPARE(thread.getCallSequence(), {"performSSLHandshakeIfNeeded"});
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_useSnakeOilCertsSetting_data()
{
    QTest::addColumn<std::optional<bool>>("useSnakeOil");
    QTest::addColumn<int>("useSnakeOilCallCount");
    QTest::addColumn<int>("loadSSLCertsCallCount");

    QTest::newRow("use SnakeOil Certs")  << std::optional<bool>{true} << 1 << 0;
    QTest::newRow("do NOT use SnakeOil Certs")  << std::optional<bool>{false}  << 0 << 1;
    QTest::newRow("default is to use SnakeOil Certs")  << std::optional<bool>{}  << 1 << 0;
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_useSnakeOilCertsSetting()
{
    QFETCH(std::optional<bool>, useSnakeOil);
    QFETCH(int, useSnakeOilCallCount);
    QFETCH(int, loadSSLCertsCallCount);

    constexpr bool shouldUseSSL = true;
    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest(), shouldUseSSL);

    if (useSnakeOil.has_value())
    {
        QSettings& settings = getSettings();
        settings.setValue(LOAD_SNAKEOIL_CERTS, useSnakeOil.value());
        settings.sync();
    }

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(thread.getCallCount(CallTracker::LOAD_SNAKEOIL_CERTS_()), useSnakeOilCallCount);
    QCOMPARE(thread.getCallCount(CallTracker::LOAD_SSL_CERTS()), loadSSLCertsCallCount);
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_ignoreSSLErrors_data()
{
    QTest::addColumn<std::optional<bool>>("ignoreSslErrorsSetting");
    QTest::addColumn<int>("ignoreSSlErrorsCallCount");

    QTest::newRow("ignoreSSLErrors setting true")  << std::optional<bool>{true} << 1;
    QTest::newRow("ignoreSSLErrors setting false")  << std::optional<bool>{false}  << 0;
    QTest::newRow("ignoreSSLErrors setting default")  << std::optional<bool>{} << 1;
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_ignoreSSLErrors()
{
    QFETCH(std::optional<bool>, ignoreSslErrorsSetting);
    QFETCH(int, ignoreSSlErrorsCallCount);

    constexpr bool shouldUseSSL = true;
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);

    if (ignoreSslErrorsSetting.has_value())
    {
        QSettings& settings = getSettings();
        settings.setValue(IGNORE_SSL_CHECK, ignoreSslErrorsSetting.value());
        settings.sync();
    }

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(sock->getCallCount(CallTracker::IGNORE_SSL_ERRORS()), ignoreSSlErrorsCallCount);
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_callStartServerEncryption()
{
    constexpr bool shouldUseSSL = true;
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(sock->getCallCount(CallTracker::START_SERVER_ENCRYPTION()), 1);
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_callWaitForEncrypted_hasNoErrors()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->setMockEncrypted(true);

    constexpr bool shouldUseSSL = true;
    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(sock->getCallCount(CallTracker::WAIT_FOR_ENCRYPTED()), 1);

    QCOMPARE(errorMessageSpy.count(), 0);
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_callWaitForEncrypted_hasErrors()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->mockSSLHandshakeShouldSucceed = false;

    constexpr bool shouldUseSSL = true;
    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(sock->getCallCount(CallTracker::WAIT_FOR_ENCRYPTED()), 1);

    QCOMPARE(errorMessageSpy.count(), 1);
    QCOMPARE(errorMessageSpy.first().first().value<QString>(), "Incoming SSL handshake failed (waitForEncrypted timeout)");
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_callWaitForEncrypted_hasErrors_doesNotCallEmitSSLDiagnosisPackets()
{
    auto sock = TestUtils::createMockSocketForTest();
    sock->mockSSLHandshakeShouldSucceed = false;

    constexpr bool shouldUseSSL = true;
    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(thread.getCallCount(CallTracker::EMIT_SSL_DIAGNOSTIC_PACKETS()), 0);
}

void IncomingTcpThreadTests::testPerformSSLHandshakeIfNeeded_successPath_callsEmitSSLDiagnosisPackets()
{
    constexpr bool shouldUseSSL = true;
    auto sock = TestUtils::createMockSocketForTest();
    sock->mockSSLHandshakeShouldSucceed = true;

    auto thread = IncomingTcpThreadTestDouble(sock, shouldUseSSL);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callPerformSSLHandshakeIfNeeded();
    QCOMPARE(thread.getCallCount(CallTracker::EMIT_SSL_DIAGNOSTIC_PACKETS()), 1);
}

// handleIncomingConnection() tests
void IncomingTcpThreadTests::testHandleIncomingConnection_socketInterfaceIsNullptr_emitsErrorMessage()
{
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.setSocketForTest(nullptr);
    thread.callHandleIncomingConnection();
    QCOMPARE(errorMessageSpy.count(), 1);
    QCOMPARE(errorMessageSpy.first().first().value<QString>(), "handleIncomingConnection: null socket interface");
}

void IncomingTcpThreadTests::testHandleIncomingConnection_emitsConnectionStatus_incomingConnectionAccepted()
{
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock);

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callHandleIncomingConnection();
    QCOMPARE(errorMessageSpy.count(), 0);
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().first().value<QString>(), "Incoming connection accepted");
}

void IncomingTcpThreadTests::testHandleIncomingConnection_successPath()
{
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock);

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callHandleIncomingConnection();
    QCOMPARE(errorMessageSpy.count(), 0);

    std::vector<QString> expectedCallSequence = {
        CallTracker::HANDLE_INCOMING_CONNECTION(),
        CallTracker::PERFORM_SSL_HANDSHAKE_IF_NEEDED(),
        CallTracker::BUILD_INITIAL_RECEIVED_PACKET(),
        CallTracker::SEND_SMART_REPLY_IF_CONFIGURED()
    };
    QCOMPARE(thread.getCallSequence(), expectedCallSequence);
}

void IncomingTcpThreadTests::testHandleIncomingConnection_successPath_emitsReceivedPacket()
{
    auto sock = TestUtils::createMockSocketForTest();
    auto thread = IncomingTcpThreadTestDouble(sock);

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy packetReceivedSpy(&thread, &BaseTcpThread::packetReceived);

    thread.callHandleIncomingConnection();
    QCOMPARE(errorMessageSpy.count(), 0);
    QCOMPARE(packetReceivedSpy.count(), 1);

    const auto returnedPacket = packetReceivedSpy.first().first().value<Packet>();
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
    QCOMPARE(normalizedPacket, p);
}

void IncomingTcpThreadTests::testRun_exitsEarly_ifSocketInterfaceIsNullPtr()
{
    auto sock = TestUtils::createMockSocketForTest();

    auto thread = IncomingTcpThreadTestDouble(sock);
    thread.setSocketForTest(nullptr);

    thread.callRun();

    // we only care about the calls made directly from run()
    const auto callSequence =  thread.getCallSequence();
    const std::vector<QString> expectedCallSequence = {CallTracker::RUN()};
    QCOMPARE(callSequence, expectedCallSequence);
}

void IncomingTcpThreadTests::testRun_callSequence()
{
    auto thread = IncomingTcpThreadTestDouble(TestUtils::createMockSocketForTest());
    thread.callRun();

    // we only care about the calls made directly from run()
    const auto callSequence =  thread.getCallSequence();
    QCOMPARE(callSequence.at(0), CallTracker::RUN());
    QCOMPARE(callSequence.at(1), CallTracker::HANDLE_INCOMING_CONNECTION());
    QCOMPARE(callSequence.back(), CallTracker::CLOSE_CONNECTION());
}
