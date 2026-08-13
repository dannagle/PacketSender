//
// Created by Tomas Gallucci on 5/9/26.
//

#include <QtTest/QTest.h>

#include <utility>

#include "outgoingtcpthreadtests.h"

#include "settingnames.h"
#include "utils/testutils.h"
#include "../../tcpThreads/outgoingtcpthread.h"
#include "../../packet.h"
#include "testdoubles/MockSslSocket.h"
#include "testdoubles/outgoingtcpthreadtestdouble.h"


// defaults are "127.0.0.1" and 9999 respectfully
Packet OutgoingTcpThreadTests::createPacketForTest(const QString& address, unsigned int port)
{
    Packet p;

    p.toIP = address;
    p.port = port;
    p.hexString = "AA BB CC DD";

    return p;
}

void OutgoingTcpThreadTests::testConstructor_throwsIfPacketToSendPortIsNotSet()
{
    Packet p = createPacketForTest(OutgoingTcpThreadTests::DEFAULT_ADDRESS, 0);

    try
    {
        OutgoingTcpThread thread = OutgoingTcpThread(p);
        QFAIL("invalid_argument exception was not thrown");
    } catch (std::invalid_argument& e)
    {
        QCOMPARE(e.what(), "OutgoingTcpThread: packetToSend.port must be set to a positive integer value");
    }
}

void OutgoingTcpThreadTests::testConstructor_throwsIfPacketToSendAddressIsNotSet()
{
    Packet p = createPacketForTest("");

    try
    {
        OutgoingTcpThread thread = OutgoingTcpThread(p);
        QFAIL("invalid_argument exception was not thrown");
    } catch (std::invalid_argument& e)
    {
        QCOMPARE(e.what(), "OutgoingTcpThread: packetToSend.toIP cannot be empty");
    }
}

void OutgoingTcpThreadTests::testConstructor_setsPersistentFlagFromPacket_data()
{
    QTest::addColumn<bool>("flagOnPacket");
    QTest::addColumn<bool>("expectedValue");

    QTest::newRow("persistent flag on packet = true")  << true  << true;
    QTest::newRow("persistent flag on packet = false")  << false  << false;
}

void OutgoingTcpThreadTests::testConstructor_setsPersistentFlagFromPacket()
{
    QFETCH(bool, flagOnPacket);
    QFETCH(bool, expectedValue);

    Packet p = TestUtils::createPacketForTest();
    p.persistent = flagOnPacket;

    OutgoingTcpThreadTestDouble thread(p);
    QCOMPARE(thread.isPersistent(), expectedValue);
}

void OutgoingTcpThreadTests::testConstructor_packetFlagNotExplicitlySetOutsideOfInitInPacket()
{
    OutgoingTcpThreadTestDouble thread(TestUtils::createPacketForTest());
    QCOMPARE(thread.isPersistent(), false);
}

void OutgoingTcpThreadTests::testGetDestinationAddress()
{
    QString destinationAddress = "foo bar baz";
    Packet p = createPacketForTest(destinationAddress);

    OutgoingTcpThread thread = OutgoingTcpThread(p);
    QCOMPARE(thread.getDestinationAddress(), destinationAddress);
}

void OutgoingTcpThreadTests::testGetDestinationPort()
{
    unsigned int port = 666;
    Packet p = createPacketForTest(OutgoingTcpThreadTests::DEFAULT_ADDRESS, port);

    OutgoingTcpThread thread = OutgoingTcpThread(p);
    QCOMPARE(thread.getDestinationPort(), port);
}

// isValid() tests
void OutgoingTcpThreadTests::testIsValid_returnsFalseWhenSendPacketDotToIpIsEmptyString()
{
    // taking advantage of createPacketForTest defaults
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(new RealQSslSocket(new QSslSocket()), createPacketForTest());

    Packet p = thread.getSendPacketByReference();
    p.port = 0;

    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpThreadTests::testIsValid_returnsFalseWhenSendPacketDotPortIsZero()
{
    // taking advantage of createPacketForTest defaults
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(createPacketForTest());

    Packet p = thread.getSendPacketByReference();
    p.toIP = "";

    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpThreadTests::testIsValid_returnsFalseWhenSocketHasNotBeenConnectedToHost()
{
    OutgoingTcpThreadTestDouble thread = OutgoingTcpThreadTestDouble(createPacketForTest());
    QCOMPARE(thread.isValid(), false);
}

void OutgoingTcpThreadTests::testIsValid_returnsTrueWithValidPacketAndSocket()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    Packet validPacket = createPacketForTest("127.0.0.1", 9999);

    OutgoingTcpThreadTestDouble thread(mockSock, validPacket);
    thread.debugSocketState();
    QCOMPARE(thread.isValid(), true);
}

// preparePacket() tests
void OutgoingTcpThreadTests::testPreparePacket()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    Packet validPassedInPacket = createPacketForTest("127.0.0.1", 9999);

    OutgoingTcpThreadTestDouble thread(mockSock, validPassedInPacket);
    thread.callPrepareOutgoingSendPacket();

    const Packet sendPreparedPacket = thread.getSendPacketByReference();
    QCOMPARE("You", sendPreparedPacket.fromIP);
    QVERIFY(sendPreparedPacket.timestamp.isValid());
    QCOMPARE(sendPreparedPacket.timestamp.toString(DATETIMEFORMAT), sendPreparedPacket.name);
}

// buildReplyPacket() tests
void OutgoingTcpThreadTests::testBuildReplyPacket_data()
{
    QTest::addColumn<bool>("isEncrypted");
    QTest::addColumn<QString>("expectedTcpOrUdp");
    QTest::addColumn<bool>("responseDataEmpty");

    QTest::newRow("plain TCP + response data")     << false << "TCP"   << false;
    QTest::newRow("SSL encrypted + response data") << true  << "SSL"   << false;
    QTest::newRow("plain TCP + empty responseData")<< false << "TCP"   << true;
}

void OutgoingTcpThreadTests::testBuildReplyPacket()
{
    QFETCH(bool, isEncrypted);
    QFETCH(QString, expectedTcpOrUdp);
    QFETCH(bool, responseDataEmpty);

    auto* mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockEncrypted(isEncrypted);

    Packet received = TestUtils::createPacketForTest();
    received.fromIP = "192.168.1.100";
    received.fromPort = 54321;
    received.timestamp = QDateTime::currentDateTime();

    QByteArray responseData;
    if (!responseDataEmpty) {
        responseData = Packet::HEXtoByteArray("AA BB CC DD");
    }
    QDEBUG() << "responseData: " << responseData;

    OutgoingTcpThreadTestDouble thread(mockSock, received);
    Packet reply = thread.callBuildReplyPacket(received, responseData);

    // === Handle timestamp in name field ===
    QVERIFY(reply.name.startsWith("Reply to "));

    QString timestampPart = reply.name.mid(9); // skip "Reply to "
    QVERIFY(QDateTime::fromString(timestampPart, DATETIMEFORMAT).isValid());

    // Normalize name for == comparison
    Packet normalizedReply = reply;
    normalizedReply.name = "Reply to ";

    Packet expectedReply;
    expectedReply.fromIP = "You (Response)";
    expectedReply.toIP = received.fromIP;
    expectedReply.port = received.fromPort;
    expectedReply.fromPort = mockSock->localPort();
    expectedReply.tcpOrUdp = expectedTcpOrUdp;
    expectedReply.hexString = Packet::byteArrayToHex(responseData);
    expectedReply.name = "Reply to ";

    QCOMPARE(normalizedReply, expectedReply);
}

// isValidForSending() tests
void OutgoingTcpThreadTests::testIsValidForSending_portIsZero()
{
    // this is not the packet we'll use for testing isValidForSending, jus the one to pass to the constructor
    const Packet validPassedInPacket = createPacketForTest("127.0.0.1", 9999);
    OutgoingTcpThreadTestDouble thread(TestUtils::createMockSocketForTest(), validPassedInPacket);

    Packet p;
    p.toIP = "";
    p.port = 0;
    p.hexString = "AA BB CC DD";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QCOMPARE(returnValue, false);
    QCOMPARE(errorMessage, "Port must be a positive number");
}

void OutgoingTcpThreadTests::testIsValidForSending_hexStringIsEmpty()
{
    const Packet validPassedInPacket = createPacketForTest("127.0.0.1", 9999);
    OutgoingTcpThreadTestDouble thread(TestUtils::createMockSocketForTest(), validPassedInPacket);

    Packet p;
    p.toIP = "";
    p.port = 9999;
    p.hexString = "";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QCOMPARE(returnValue, false);
    QCOMPARE(errorMessage, "No data to send (hexString is empty)");
}

void OutgoingTcpThreadTests::testIsValidForSending_toIPIsEmpty()
{
    const Packet validPassedInPacket = createPacketForTest("127.0.0.1", 9999);
    OutgoingTcpThreadTestDouble thread(TestUtils::createMockSocketForTest(), validPassedInPacket);

    Packet p;
    p.toIP = "";
    p.port = 9999;
    p.hexString = "not an empty hex string";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QCOMPARE(returnValue, false);
    QCOMPARE(errorMessage, "Destination address (toIP) is empty");
}

void OutgoingTcpThreadTests::testIsValidForSending_happyPath()
{
    const Packet validPassedInPacket = createPacketForTest("127.0.0.1", 9999);
    OutgoingTcpThreadTestDouble thread(TestUtils::createMockSocketForTest(), validPassedInPacket);

    Packet p;
    p.toIP = "127.0.0.1";
    p.port = 9999;
    p.hexString = "AA BB CC DD";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QVERIFY(errorMessage.isEmpty());
    QCOMPARE(returnValue, true);
}

// shouldSendReply() tests
void OutgoingTcpThreadTests::testShouldSendReply_data()
{
    QTest::addColumn<bool>("consoleMode");
    QTest::addColumn<bool>("basicEnabled");
    QTest::addColumn<bool>("smartEnabled");
    QTest::addColumn<bool>("hasCommandLineReply");
    QTest::addColumn<bool>("expected");

    // Console mode cases
    QTest::newRow("console + nothing")           << true  << false << false << false << false;
    QTest::newRow("console + basic")             << true  << true  << false << false << false;
    QTest::newRow("console + smart")             << true  << false << true  << false << false;
    QTest::newRow("console + command line only") << true  << false << false << true  << true;
    QTest::newRow("console + all")               << true  << true  << true  << true  << true;

    // Normal (non-console) cases
    QTest::newRow("normal + nothing")            << false << false << false << false << false;
    QTest::newRow("normal + basic")              << false << true  << false << false << true;
    QTest::newRow("normal + smart")              << false << false << true  << false << true;
    QTest::newRow("normal + command line")       << false << false << false << true  << true;
    QTest::newRow("normal + basic + smart")      << false << true  << true  << false << true;
    QTest::newRow("normal + all")                << false << true  << true  << true  << true;
}

void OutgoingTcpThreadTests::testShouldSendReply()
{
    QFETCH(bool, consoleMode);
    QFETCH(bool, basicEnabled);
    QFETCH(bool, smartEnabled);
    QFETCH(bool, hasCommandLineReply);
    QFETCH(bool, expected);

    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.setConsoleMode(consoleMode);

    // Simulate settings
    QSettings &settings = getSettings();
    settings.setValue("sendReponse", basicEnabled);
    settings.setValue("smartResponseEnableCheck", smartEnabled);
    settings.sync();

    // Simulate command line reply
    if (hasCommandLineReply) {
        thread.getCommandLineReplyPacketByReference().hexString = "AA BB CC DD";
    } else {
        thread.getCommandLineReplyPacketByReference().hexString.clear();
    }

    QCOMPARE(thread.callShouldSendReply(), expected);
}

// sendReplyIfNeeded() tests
void OutgoingTcpThreadTests::testSendReplyIfNeeded_exitsEarlyIfShouldSendReplyReturnsFalse()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    auto p = TestUtils::createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);
    thread.setConsoleMode(false);

    // Simulate settings
    QSettings &settings = getSettings();
    settings.setValue("sendReponse", false);
    settings.setValue("smartResponseEnableCheck", false);
    settings.sync();

    thread.callSendReplyIfNeeded(p);

    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back("sendReplyIfNeeded");
    expectedCallSequence.push_back("shouldSendReply");
    QCOMPARE(thread.getCallSequence(), expectedCallSequence);
}

OutgoingTcpThreadTestDouble& setupThreadToSendReply()
{
    const auto mockSock = TestUtils::createMockSocketForTest();
    const auto p = TestUtils::createPacketForTest();
    auto thread = new OutgoingTcpThreadTestDouble(mockSock, p);
    thread->setConsoleMode(false);

    // not all flags need to be true to send a response.
    // But if we set them all to true, we can turn them off in tets
    QSettings &settings = getSettings();
    settings.setValue(SEND_RESPONSE, true);
    settings.setValue(SMART_RESPONSES_ENABLED, true);
    settings.sync();

    return *thread;
}

Packet buildExpectedReplyPacket(OutgoingTcpThreadTestDouble& thread, const unsigned int port, const QString& responseHex)
{
    auto p = thread.getSendPacketByReference();
    p.name = "Reply to";
    p.port = port;
    p.fromPort = 0;
    p.fromIP = "You (Response)";
    p.hexString = responseHex;
    p.hexString += " ";

    return p;
}

bool normalizeReplyPacket(const Packet& returnedPacket, Packet& outPacket, QString& outErrorMessage)
{
    if (!returnedPacket.name.startsWith("Reply to"))
    {
        outErrorMessage = "packet name did not start with 'Reply to'";
        return false;
    }

    QString timestampPart = returnedPacket.name.mid(9);

    if (!QDateTime::fromString(timestampPart, DATETIMEFORMAT).isValid())
    {
        outErrorMessage = "packet name did not contain a valid timestamp";
        return false;
    }

    outPacket = returnedPacket;
    outPacket.name = "Reply to";

    return true;
}

std::vector<QString> getSendReplyIfNeededExpectedCallSequence()
{
    std::vector<QString> expectedCallSequence;
    expectedCallSequence.push_back(CallTracker::SEND_REPLY_IF_NEEDED());
    expectedCallSequence.push_back(CallTracker::SHOULD_SEND_REPLY());
    expectedCallSequence.push_back(CallTracker::BUILD_REPLY_PACKET());
    expectedCallSequence.push_back(CallTracker::GET_SMART_RESPONSE_DATA());
    expectedCallSequence.push_back(CallTracker::OUTGOINGTCPTHREAD_ISVALIDFORSENDING());
    // because BaseTcpThread::sendOutgoingPacket is called via a qualified call,
    // our override isn't called, so we don't get the sequence here
    return expectedCallSequence;
}

void OutgoingTcpThreadTests::testSendReplyIfNeeded_sendsPacket_whenResponseHexIsSet()
{
    auto &settings = getSettings();
    constexpr auto responseHex = "FF DD EE 00 11 22";
    settings.setValue(RESPONSE_HEX, responseHex);
    settings.sync();

    OutgoingTcpThreadTestDouble& thread = setupThreadToSendReply();

    QSignalSpy errorSpy(&thread, &BaseTcpThread::error);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callSendReplyIfNeeded(thread.getSendPacketByReference());

    std::vector<QString> expectedCallSequence = getSendReplyIfNeededExpectedCallSequence();
    QCOMPARE(thread.getCallSequence(), expectedCallSequence);

    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(errorMessageSpy.count(), 0);
    QCOMPARE(packetSentSpy.count(), 1);

    auto returnedPacket = packetSentSpy.first().first().value<Packet>();

    Packet normalizedReply;
    QString errorMessage;
    QVERIFY2(normalizeReplyPacket(
        returnedPacket, normalizedReply, errorMessage),
        qPrintable(errorMessage.prepend("failed to normalize packet\n")));

    auto p = buildExpectedReplyPacket(thread, normalizedReply.port, responseHex);
    QCOMPARE(normalizedReply, p);
}

void OutgoingTcpThreadTests::testSendReplyIfNeeded_sendsPacket_commandlineOverrides()
{
    auto &settings = getSettings();
    constexpr auto responseHex = "FF DD EE 00 11 22";
    settings.setValue(RESPONSE_HEX, responseHex);
    settings.sync();

    OutgoingTcpThreadTestDouble& thread = setupThreadToSendReply();
    thread.getCommandLineReplyPacketByReference().toIP = "128.0.0.1";
    thread.getCommandLineReplyPacketByReference().hexString = "foo bar baz";

    QSignalSpy errorSpy(&thread, &BaseTcpThread::error);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callSendReplyIfNeeded(thread.getSendPacketByReference());

    std::vector<QString> expectedCallSequence = getSendReplyIfNeededExpectedCallSequence();
    QCOMPARE(thread.getCallSequence(), expectedCallSequence);

    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(errorMessageSpy.count(), 0);
    QCOMPARE(packetSentSpy.count(), 1);

    auto returnedPacket = packetSentSpy.first().first().value<Packet>();
    QCOMPARE(returnedPacket, thread.getCommandLineReplyPacketByReference());
}


void OutgoingTcpThreadTests::testSendReplyIfNeeded_doesNOTSendPacket_whenNoResponse()
{
    auto &settings = getSettings();
    settings.remove(RESPONSE_HEX);
    settings.sync();

    OutgoingTcpThreadTestDouble& thread = setupThreadToSendReply();

    QSignalSpy errorSpy(&thread, &BaseTcpThread::error);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callSendReplyIfNeeded(thread.getSendPacketByReference());

    std::vector<QString> expectedCallSequence = getSendReplyIfNeededExpectedCallSequence();
    expectedCallSequence.pop_back();
    QCOMPARE(thread.getCallSequence(), expectedCallSequence);

    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(errorMessageSpy.count(), 0);
    QCOMPARE(packetSentSpy.count(), 0);
}

// getSmartResponseData() tests
void OutgoingTcpThreadTests::testGetSmartResponseData_data()
{
    QTest::addColumn<bool>("smartEnabled");
    QTest::addColumn<QString>("incomingHex");
    QTest::addColumn<QString>("expectedSmartReplyHex");  // empty = no match / disabled

    QTest::newRow("smart disabled")
        << false << "AA BB CC" << "";

    QTest::newRow("smart enabled - no match")
        << true << "00 11 22" << "";

    // You can add more rows once you have real smart config test data
    QTest::newRow("smart enabled - match found")
        << true << "DE AD BE EF" << "CA FE BA BE ";
}

void OutgoingTcpThreadTests::testGetSmartResponseData()
{
    QFETCH(bool, smartEnabled);
    QFETCH(QString, incomingHex);
    QFETCH(QString, expectedSmartReplyHex);

    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.setConsoleMode(false);

    // Setup settings
    QSettings &settings = getSettings();
    settings.setValue(SMART_RESPONSES_ENABLED, smartEnabled);

    if (smartEnabled) {
        // Setup Smart Response #1 (the one we'll match on)
        settings.setValue("responseEnableCheck1", true);
        settings.setValue("responseIfEdit1", "DE AD BE EF");
        settings.setValue("responseReplyEdit1", "CA FE BA BE");
        settings.setValue("matchMethodBox1", "Exact Match");
        settings.setValue("responseEncodingBox1", "HEX");
        // You can leave the other 4 empty
    }

    settings.sync();

    Packet received = TestUtils::createPacketForTest();
    received.hexString = incomingHex;

    QByteArray result = thread.callGetSmartResponseData(received);

    if (expectedSmartReplyHex.isEmpty()) {
        QVERIFY(result.isEmpty());
    } else {
        QCOMPARE(Packet::byteArrayToHex(result), expectedSmartReplyHex);
    }

    // Optional: check call tracking
    const auto& calls = thread.getCallSequence();
    QVERIFY(std::find(calls.begin(), calls.end(), "getSmartResponseData") != calls.end());
}
