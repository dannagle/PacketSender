//
// Created by Tomas Gallucci on 5/19/26.
//

#ifndef SINGLESENDOUTGOINGTCPTHREADTESTS_H
#define SINGLESENDOUTGOINGTCPTHREADTESTS_H
#include <QObject>


class SingleSendOutgoingTcpThreadTests : public QObject
{
    Q_OBJECT

    private slots:
    void testRun_SocketSuccessfullyConnected_emitsConnectionStatus_Connected();
    void testRun_SocketSuccessfullyConnected_emitsConnectionStatus_SendingData();
    void testRun_SocketSuccessfullyConnected_emitsConnectionStatus_Disconnected();

    void testRun_SocketNotConnected_emitsConnectionStatus_CouldNotConnect();
    void testRun_SocketNotConnected_packetErrorMessageUpdated();
    void testRun_SocketNotConnected_emitsPacketSent();
    void testRun_SocketNotConnected_emitsErrorMessage();
    void testRun_successPath_callsMethodsInCorrectOrder();
    void testRun_respectsDelayAfterConnect();
    void testSingleShot_sendsSmartResponse();
};


#endif //SINGLESENDOUTGOINGTCPTHREADTESTS_H
