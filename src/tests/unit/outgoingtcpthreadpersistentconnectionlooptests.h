//
// Created by Tomas Gallucci on 5/19/26.
//

#ifndef OUTGOINGTCPTHREADPERSISTENTCONNECTIONLOOPTESTS_H
#define OUTGOINGTCPTHREADPERSISTENTCONNECTIONLOOPTESTS_H

#include <QtTest/QTest>


class OutgoingTcpThreadPersistentConnectionLoopTests : public QObject
{
    Q_OBJECT

private slots:

    // shouldContinuePersistentLoop() tests
    void testShouldContinuePersistentLoop_returnsTrueWhenAllConditionsMet();
    void testShouldContinuePersistentLoop_returnsFalseWhenInterruptionRequested();
    void testShouldContinuePersistentLoop_returnsFalseWhenNoSocket();
    void testShouldContinuePersistentLoop_returnsFalseWhenSocketNotConnected();

    // shouldStopPersistentLoop() tests
    void testShouldStopPersistentLoop_returnsFalseWhenStopHasBeenCalled();
    void testShouldStopPersistentLoop_returnsFalseWhenStopHasNotBeenCalled();

    // handlePersistentIdleCase() tests
    void testHandlePersistentIdleCase_emitsWhenThresholdPassed();
    void testHandlePersistentIdleCase_doesNotEmitTooSoon();
    void testHandlePersistentIdleCase_respectsCustomThreshold();

    // buildReceivedPacket() tests
    void testBuildReceivedPacket_socketNull();
    void testBuildReceivedPacket_socketHasNoData();
    void testBuildReceivedPacket_socketHasData();

    // processIncomingData() tests
    void testProcessIncomingData_socketIsNull_returnsEarly();
    void testProcessIncomingData_socketHasNoData_returnsEarly();
    void testProcessIncomingData_socketHasData_emitsReceivedPacket();
    void testProcessIncomingData_socketHasData_callsSendResponseIfNeeded();

    // waitForAndProcessIncomingData() tests
    void testWaitForAndProcessIncomingData_emitsConnectionStatus_WaitingForDataBeforeSend();
    void testWaitForAndProcessIncomingData_functionCallsOrder();

    // persistentConnectionLoop() tests
    void testPersistentConnectionLoop_exitsImmediatelyOnInterruption();
    void testPersistentConnectionLoop_callsShouldContinuePersistentLoop();
    void testPersistentConnectionLoop_callsShouldStopPersistentLoop();
    void testPersistentConnectionLoop_callsIdleHandlerWhenNoData();
    void testPersistentConnectionLoop_skipsIdleWhenHasDataToSend();
    void testPersistentConnectionLoop_exitsLoopEarlyIfAlreadyStopped();
    void testPersistentConnectionLoop_exitsWhenSocketDisconnects();
    void testPersistentConnectionLoop_emitsIdleStatusOnlyEveryTwoSeconds();
    void testPersistentConnectionLoop_callsProcessIncomingData();
    void testPersistentConnectionLoop_callsWaitForAndProcessIncomingData();
    void testPersistent_sendsSmartResponse();

    // run() + persistent tests
    void testRun_callsPersistentConnectionLoopWhenFlagIsSet();
    void testRun_closesConnectionAfterPersistentLoopExits();
    void testRun_SSL_persistent_worksRepeatedly();
};


#endif //OUTGOINGTCPTHREADPERSISTENTCONNECTIONLOOPTESTS_H
