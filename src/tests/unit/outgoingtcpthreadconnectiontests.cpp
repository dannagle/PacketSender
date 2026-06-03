//
// Created by Tomas Gallucci on 5/30/26.
//

#include "outgoingtcpthreadconnectiontests.h"

#include "fileutils.h"
#include "settingnames.h"
#include "testutils.h"
#include "testdoubles/outgoingtchpthreadtestdouble.h"
#include "testdoubles/MockSslSocket.h"

void OutgoingTcpThreadConnectionTests::testHandleOutgoingPlainTCP_callsConnectToHost()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->shouldCallConnectToHost = false;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QVERIFY(thread.callHandleOutgoingPlainTCP());

    QCOMPARE(mockSock->connectToHostCallCount, 1);
    QCOMPARE(mockSock->lastConnectedToHostName, "127.0.0.1");  // or whatever default
    QCOMPARE(mockSock->lastConnectedToPort, 666);
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingPlainTCP_emitsSuccess()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->shouldCallConnectToHost = false;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy connectionStatusSpy(&thread,  &BaseTcpThread::connectionStatus);
    QVERIFY(thread.callHandleOutgoingPlainTCP());

    QCOMPARE(connectionStatusSpy.last().at(0).value<QString>(), "Connected");
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingPlainTCP_callsHandleConnectionFailure()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->shouldCallConnectToHost = false;
    mockSock->makeWaitForConnectedReturnFalse = true;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QVERIFY(!thread.callHandleOutgoingPlainTCP());
    QVERIFY(thread.wasMethodCalled("handleConnectionFailure"));
}

// handleConnectionFailure() tests
void OutgoingTcpThreadConnectionTests::testHandleConnectionFailure_emitsConnectionStatus_CouldNotConnect()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callHandleConnectionFailure();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.last().at(0).value<QString>(), "Could not connect.");
}

void OutgoingTcpThreadConnectionTests::testHandleConnectionFailure_emitsErrorMessage()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    auto p = TestUtils::createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callHandleConnectionFailure();
    QCOMPARE(errorMessageSpy.count(), 1);

    const QString expectedErrorMessage = "Could not connect to " + p.toIP + ":" + QString::number(p.port);
    QCOMPARE(errorMessageSpy.last().at(0).value<QString>(), expectedErrorMessage);
}

void OutgoingTcpThreadConnectionTests::testHandleConnectionFailure_emitsPacketSent()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    auto p = TestUtils::createPacketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, p);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callHandleConnectionFailure();
    QCOMPARE(packetSentSpy.count(), 1);

    p.errorString = "Could not connect";
    QCOMPARE(packetSentSpy.last().at(0).value<Packet>(), p);
}

// loadSnakeOilCerts() tests
void OutgoingTcpThreadConnectionTests::testLoadSnakeOilCerts_loadsCerts()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.callLoadSSLCerts(true);   // allowSnakeOil = true

    QCOMPARE(mockSock->setLocalCertificateCallCount, 1);
    QCOMPARE(mockSock->setPrivateKeyCallCount, 1);

    const QSslCertificate& loadedCert = mockSock->mockLocalCertificate;

    QVERIFY(!loadedCert.isNull());
    QCOMPARE(loadedCert.issuerInfo(QSslCertificate::CommonName).join(""), "SnakeOil");
    QCOMPARE(loadedCert.subjectInfo(QSslCertificate::CommonName).join(""), "SnakeOil");
}

void OutgoingTcpThreadConnectionTests::testLoadSSLCerts_productionPathsMissing()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    const QString nonexistentCertPath = "/nonexistent/cert.pem";
    const QString nonexistentKeyPath = "/nonexistent/private.key";

    // Setup production mode with non-existent files
    QSettings& settings = getSettings();
    settings.setValue(LOAD_SNAKEOIL_CERTS, false);
    settings.setValue(SET_LOCAL_CERTIFICATE_PATH, nonexistentCertPath);
    settings.setValue(SSL_PRIVATE_KEY_PATH, nonexistentKeyPath);
    settings.sync();

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callLoadSSLCerts(false);

    // Should NOT fall back to snake oil
    QVERIFY(!thread.wasMethodCalled("loadSnakeOilCerts"));

    // Should emit error messages about the missing files
    QCOMPARE(errorMessageSpy.count(), 2);

    QCOMPARE(errorMessageSpy.first().at(0).value<QString>(), "SSL: Failed to load certificate from: " + nonexistentCertPath);
    QCOMPARE(errorMessageSpy.last().at(0).value<QString>(), "SSL: Failed to load private key from: " + nonexistentKeyPath);
}

// loadSSLCerts() tests

void OutgoingTcpThreadConnectionTests::testLoadSSLCerts_exitsEarlyIfSocketInterfaceIsNull()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.setSocketForTest(nullptr);
    thread.callLoadSSLCerts(true); // the value of the boolean doesn't matter here
    QCOMPARE(errorMessageSpy.count(), 1);
    QCOMPARE(errorMessageSpy.last().at(0).value<QString>(), "loadSSLCerts called with null socketInterface");
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_failure()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(false);   // simulate connection failure
    mockSock->makeWaitForConnectedReturnFalse = true;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    bool success = thread.callHandleOutgoingSSL();
    QVERIFY(!success);

    // Should have called failure handler
    QVERIFY(thread.wasMethodCalled("handleOutgoingSSLHandshakeFailure"));

    // Should have emitted failure messages
    QVERIFY(errorMessageSpy.count() > 0);
}

void OutgoingTcpThreadConnectionTests::testLoadSSLCerts_calls_loadsSnakeOilCerts()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callLoadSSLCerts(true);
    auto sequence = thread.getCallSequence();
    QCOMPARE(sequence.back(), "loadSnakeOilCerts");
}

// Helper function - take const references

QString getHandleOutgoingSSLHandshakeSuccessErrorMessage(const QString& expectedBeginsWith,
                                                         const QString& actualString)
{
    return "Expected errorMessage to begin with \"" + expectedBeginsWith
           + "\" but got: \"" + actualString + "\"";
}

// handleOutgoingSSLHandshakeSuccess() tests

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSLHandshakeSuccess()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    thread.callHandleOutgoingSSLHandshakeSuccess();

    QCOMPARE(packetSentSpy.count(), 3);

    const QString firstMessage = packetSentSpy.at(0).at(0).value<Packet>().errorString;
    const QString firstExpected = "Encrypted with ";
    QVERIFY2(firstMessage.startsWith(firstExpected),
             qPrintable(getHandleOutgoingSSLHandshakeSuccessErrorMessage(firstExpected, firstMessage)));

    const QString secondMessage = packetSentSpy.at(1).at(0).value<Packet>().errorString;
    const QString secondExpected = "Authenticated with ";
    QVERIFY2(secondMessage.startsWith(secondExpected),
             qPrintable(getHandleOutgoingSSLHandshakeSuccessErrorMessage(secondExpected, secondMessage)));

    const QString thirdMessage = packetSentSpy.at(2).at(0).value<Packet>().errorString;
    const QString thirdExpected = "Peer Cert: ";
    QVERIFY2(thirdMessage.startsWith(thirdMessage),
             qPrintable(getHandleOutgoingSSLHandshakeSuccessErrorMessage(thirdMessage, thirdMessage)));
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSLHandshakeFailure()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    // Simulate some SSL handshake errors
    mockSock->setMockSslHandshakeErrors({
        QSslError(QSslError::SelfSignedCertificate),
        QSslError(QSslError::HostNameMismatch)
    });

    thread.callHandleOutgoingSSLHandshakeFailure();

    // Should emit specific error messages + a summary
    QCOMPARE(errorMessageSpy.count(), 3);        // 2 specific + 1 summary

    // Check specific SSL errors were emitted
    QString msg1 = errorMessageSpy.at(0).at(0).value<QString>();
    QString msg2 = errorMessageSpy.at(1).at(0).value<QString>();
    QVERIFY(msg1.contains("SelfSignedCertificate") || msg1.contains("SSL Error"));
    QVERIFY(msg2.contains("HostNameMismatch") || msg2.contains("SSL Error"));

    // Check summary message
    QString summary = errorMessageSpy.at(2).at(0).value<QString>();
    QCOMPARE(summary, "SSL Handshake Failed");

    // Should also emit at least one packetSent with error info
    QVERIFY(packetSentSpy.count() >= 1);
}

// handleOutgoingSSL() tests

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_success()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    // Make sure we have a packet that wants SSL
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    bool success = thread.callHandleOutgoingSSL();

    QVERIFY(success);
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().at(0).value<QString>(), "SSL Connected");

    // Should have called the handshake success handler
    QVERIFY(thread.wasMethodCalled("handleOutgoingSSLHandshakeSuccess"));
    QVERIFY(thread.wasMethodCalled("loadSSLCerts"));

    // Should have loaded snake oil certs by default
    QVERIFY(mockSock->setLocalCertificateCallCount > 0);
    QVERIFY(mockSock->setPrivateKeyCallCount > 0);
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_callsLoadSSLCertsWithSettingsValue_data()
{
    QTest::addColumn<bool>("useSnakeOil");
    QTest::addColumn<bool>("expectedValuePassedToLoadSSLCerts");

    QTest::newRow("use snake oil = true")  << true  << true;
    QTest::newRow("use snake oil = false") << false << false;
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_callsLoadSSLCertsWithSettingsValue()
{
    QFETCH(bool, useSnakeOil);
    QFETCH(bool, expectedValuePassedToLoadSSLCerts);

    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    // Setup the setting
    QSettings& settings = getSettings();
    settings.setValue(LOAD_SNAKEOIL_CERTS, useSnakeOil);
    settings.sync();

    QCOMPARE(thread.lastAllowSnakeOilValue.has_value(), false);

    bool success = thread.callHandleOutgoingSSL();
    QVERIFY(success);

    QCOMPARE(thread.lastAllowSnakeOilValue.has_value(), true);
    QCOMPARE(thread.lastAllowSnakeOilValue.value(), expectedValuePassedToLoadSSLCerts);
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_callsLoadSSLCertsWithDefaultValueWhenSettingNotPresent()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    // Setup the setting
    QSettings& settings = getSettings();
    settings.remove(LOAD_SNAKEOIL_CERTS);
    settings.sync();

    QCOMPARE(thread.lastAllowSnakeOilValue.has_value(), false);

    bool success = thread.callHandleOutgoingSSL();
    QVERIFY(success);

    QCOMPARE(thread.lastAllowSnakeOilValue.has_value(), true);
    QCOMPARE(thread.lastAllowSnakeOilValue.value(), true);
}


void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_callsIgnoreSSLCheck_IgnoreSSLCheckSettingIsTrue()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    // Test with ignoreSSLCheck = true (default)
    QSettings &settings = getSettings();
    settings.setValue(IGNORE_SSL_CHECK, true);
    settings.sync();

    thread.callHandleOutgoingSSL();
    QCOMPARE(mockSock->ignoreSslErrorsCallCount, 1);
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_doesNoCallIgnoreSSLCheck_IgnoreSSLCheckSettingIsFalse()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    QSettings &settings = getSettings();
    settings.setValue(IGNORE_SSL_CHECK, false);
    settings.sync();

    thread.callHandleOutgoingSSL();
    QCOMPARE(mockSock->ignoreSslErrorsCallCount, 0);
}

void OutgoingTcpThreadConnectionTests::testHandleOutgoingSSL_callsLoadSSLCerts()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    // Make this an SSL packet
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    thread.callHandleOutgoingSSL();

    // Should have called loadSSLCerts with allowSnakeOil = true
    QVERIFY(thread.wasMethodCalled("loadSSLCerts"));
}

void OutgoingTcpThreadConnectionTests::testLoadSSLCerts_usesProductionCertsWhenSnakeOilIsDisabled()
{
    TestUtils::setupProductionSnakeOilCertsForTest();

    auto mockSock = TestUtils::createMockSocketForTest();
    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());

    thread.callLoadSSLCerts(false);
    QVERIFY(!thread.wasMethodCalled("loadSnakeOilCerts"));

    QCOMPARE(mockSock->setLocalCertificateCallCount, 1);
    QCOMPARE(mockSock->setPrivateKeyCallCount, 1);
}

void OutgoingTcpThreadConnectionTests::testRun_SSL_success()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callRun();

    QVERIFY(thread.wasMethodCalled("handleOutgoingSSL"));
    QCOMPARE(connectionStatusSpy.count(), 3);  // Connected, Sending data, Disconnected
    QCOMPARE(connectionStatusSpy.at(0).at(0).value<QString>(), "SSL Connected");
}

void OutgoingTcpThreadConnectionTests::testRun_SSL_handshakeFailure()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(false);
    mockSock->makeWaitForConnectedReturnFalse = true;

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    thread.callRun();

    QVERIFY(!thread.wasMethodCalled("handleOutgoingPlainTCP"));
    QVERIFY(thread.wasMethodCalled("handleOutgoingSSL"));
    QVERIFY(errorMessageSpy.count() > 0);
}

void OutgoingTcpThreadConnectionTests::testRun_SSL_callsMethodsInCorrectOrder()
{
    auto mockSock = TestUtils::createMockSocketForTest();
    mockSock->setMockConnected(true);
    mockSock->setMockEncrypted(true);

    OutgoingTcpThreadTestDouble thread(mockSock, TestUtils::createPacketForTest());
    thread.getSendPacketByReference().tcpOrUdp = "SSL";

    thread.callRun();

    const auto& callSequence = thread.getCallSequence();

    const std::vector<QString> expected = {
        "handleOutgoingSSL",
        "loadSSLCerts",
        "loadSnakeOilCerts",
        "handleOutgoingSSLHandshakeSuccess",
        "prepareOutgoingPacket",
        "sendOutgoingPacket",
        "processIncomingData",
        "closeConnection"
    };
    QCOMPARE(callSequence, expected);
}
