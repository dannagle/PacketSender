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
    // constructor tests
    void testConstructor_assignsSocketDescriptor();
    void testConstructor_assignsIsSecure();

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

private:
    static constexpr int TEST_PORT_NUMBER = 666;
};


#endif //INCOMINGTCPTHREADTESTS_H
