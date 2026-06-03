//
// Created by Tomas Gallucci on 5/30/26.
//

#ifndef OUTGOINGTCPTHREADCONNECTIONTESTS_H
#define OUTGOINGTCPTHREADCONNECTIONTESTS_H
#include <QObject>


class OutgoingTcpThreadConnectionTests : public QObject
{
    Q_OBJECT

private slots:
    // handleOutgoingPlainTCP() tests
    void testHandleOutgoingPlainTCP_callsConnectToHost();
    void testHandleOutgoingPlainTCP_emitsSuccess();
    void testHandleOutgoingPlainTCP_callsHandleConnectionFailure();

    // handleConnectionFailure() tests
    void testHandleConnectionFailure_emitsConnectionStatus_CouldNotConnect();
    void testHandleConnectionFailure_emitsErrorMessage();
    void testHandleConnectionFailure_emitsPacketSent();

    // loadSnakeOilCerts() tests
    void testLoadSnakeOilCerts_loadsCerts();
    void testLoadSSLCerts_productionPathsMissing();

    // test loadSSLCerts() tests
    void testLoadSSLCerts_exitsEarlyIfSocketInterfaceIsNull();
    void testLoadSSLCerts_calls_loadsSnakeOilCerts();
    void testLoadSSLCerts_usesProductionCertsWhenSnakeOilIsDisabled();

    // handleOutgoingSSLHandshakeSuccess() tests
    void testHandleOutgoingSSLHandshakeSuccess();

    // handleOutgoingSSLHandshakeFailure() tests
    void testHandleOutgoingSSLHandshakeFailure();

    // handleOutgoingSSL() tests
    void testHandleOutgoingSSL_success();
    void testHandleOutgoingSSL_failure();
    void testHandleOutgoingSSL_callsIgnoreSSLCheck_IgnoreSSLCheckSettingIsTrue();
    void testHandleOutgoingSSL_doesNoCallIgnoreSSLCheck_IgnoreSSLCheckSettingIsFalse();

    void testHandleOutgoingSSL_callsLoadSSLCerts();
    void testHandleOutgoingSSL_callsLoadSSLCertsWithSettingsValue_data();
    void testHandleOutgoingSSL_callsLoadSSLCertsWithSettingsValue();
    void testHandleOutgoingSSL_callsLoadSSLCertsWithDefaultValueWhenSettingNotPresent();

    // run() SSL tests
    void testRun_SSL_success();
    void testRun_SSL_handshakeFailure();
    void testRun_SSL_callsMethodsInCorrectOrder();

    // run() tests
    void testRun_nonPersistent_doesNotCallPersistentLoop();
};


#endif //OUTGOINGTCPTHREADCONNECTIONTESTS_H
