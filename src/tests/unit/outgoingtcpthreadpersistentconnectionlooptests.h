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

    // persistentConnectionLoop() tests
    void testPersistentConnectionLoop_exitsImmediatelyOnInterruption();
    void testPersistentConnectionLoop_callsShouldContinuePersistentLoop();
    void testPersistentConnectionLoop_callsShouldStopPersistentLoop();
    void testPersistentConnectionLoop_callsIdleHandlerWhenNoData();
    void testPersistentConnectionLoop_skipsIdleWhenHasDataToSend();
    void testPersistentConnectionLoop_doesNotEnterLoopIfAlreadyStopped();
    void testPersistentConnectionLoop_exitsWhenSocketDisconnects();
    void testPersistentConnectionLoop_emitsIdleStatusOnlyEveryTwoSeconds();
};


#endif //OUTGOINGTCPTHREADPERSISTENTCONNECTIONLOOPTESTS_H
