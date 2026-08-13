//
// Created by Tomas Gallucci on 6/8/26.
//

#ifndef INCOMINGTCPTHREADTESTS_H
#define INCOMINGTCPTHREADTESTS_H
#include <QObject>


class IncomingTcpThreadTests : public QObject
{
    Q_OBJECT

private slots:
    void init();

    // constructor tests
    void testConstructor_assignsSocketDescriptor();
    void testConstructor_assignsIsSecure();
    void testConstructor_assignsPersistent();

    void testBuildInitialReceivedPacket_socketInterfaceIsNullptr_data();

    // buildInitialReceivedPacket() tests
    void testBuildInitialReceivedPacket_socketInterfaceIsNullptr();
    void testBuildInitialReceivedPacket_socketInterfaceIsInvalid();
    void testBuildInitialReceivedPacket_socketInterfaceIsNotNullptr();
    void testBuildInitialReceivedPacket_socketInterfaceState_isNotConnected();

    // sendSmartReplyIfConfigured() tests
    void testSendSmartReplyIfConfigured_SendResponseSetting_isFalse();
    void testSendSmartReplyIfConfigured_SendResponseSetting_defaultValue_isFalse();
    void testSendSmartReplyIfConfigured_ResponseHexSetting_isEmptyString();
    void testSendSmartReplyIfConfigured_ResponseHexSetting_defaultValue_isEmptyString();
    void testSendSmartReplyIfConfigured_successPath();
    void testSendSmartReplyIfConfigured_successPath_withMacroExpansion();

    // emitSSLDiagnosticPackets() tests
    void testEmitSSLDiagnosticPackets_socketNullptr_emits0SentPackets();
    void testEmitSSLDiagnosticPackets_socketNotEncrypted_emits0SentPackets();
    void testEmitSSLDiagnosticPackets_successPath();
    void testPerformSSLHandshakeIfNeeded_shouldUseSslIsFalse_doesNotCallSSLMethods();

    // performSSLHandshakeIfNeeded() tests
    void testPerformSSLHandshakeIfNeeded_socketInterfaceIsNullptr_emitsErrorMessage();
    void testPerformSSLHandshakeIfNeeded_useSnakeOilCertsSetting_data();
    void testPerformSSLHandshakeIfNeeded_useSnakeOilCertsSetting();
    void testPerformSSLHandshakeIfNeeded_ignoreSSLErrors_data();
    void testPerformSSLHandshakeIfNeeded_ignoreSSLErrors();
    void testPerformSSLHandshakeIfNeeded_callStartServerEncryption();
    void testPerformSSLHandshakeIfNeeded_callWaitForEncrypted_hasNoErrors();
    void testPerformSSLHandshakeIfNeeded_callWaitForEncrypted_hasErrors();
    void testPerformSSLHandshakeIfNeeded_callWaitForEncrypted_hasErrors_doesNotCallEmitSSLDiagnosisPackets();
    void testPerformSSLHandshakeIfNeeded_successPath_callsEmitSSLDiagnosisPackets();

    // handleIncomingConnection() tests
    void testHandleIncomingConnection_socketInterfaceIsNullptr_emitsErrorMessage();
    void testHandleIncomingConnection_emitsConnectionStatus_incomingConnectionAccepted();
    void testHandleIncomingConnection_successPath();
    void testHandleIncomingConnection_successPath_emitsReceivedPacket();
    void testHandleIncomingConnection_emptyData_doesNOTemitReceivedPacket();
    void testRun_exitsEarly_ifSocketInterfaceIsNullPtr();

    // processReceivedPacket() tests
    void testProcessReceivedPacket_doesNOTEmitPacketReceived_whenHexStringIsEmpty();
    void testProcessReceivedPacket_doesNOTcallSendSmartReplyIfConfigured();
    void testProcessReceivedPacket_happyPath_emitsPacketReceived();
    void testProcessReceivedPacket_happyPath_callsSendSmartReplyIfConfigured();


    // run() tests
    void testRun_callSequence();
    void testRun_callSequence_withPersistence();

    // persistentConnectionLoop() tests
    void testPersistentConnectionLoop_socketInterfaceIsNullptr();
    void testPersistentConnectionLoop_successPath();
    void testPersistentConnectionLoop_successPath_emitsReceivedPacket();

private:
    static constexpr int TEST_PORT_NUMBER = 666;
};


#endif //INCOMINGTCPTHREADTESTS_H
