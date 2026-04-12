//
// Created by Tomas Gallucci on 4/11/26.
//

#ifndef PERSISTENTCONNECTIONLOOPTESTS_H
#define PERSISTENTCONNECTIONLOOPTESTS_H

#include <QObject>

class PersistentConnectionLoopTests : public QObject
{
    Q_OBJECT

private slots:
    void testPrepareForPersistentLoop_preparesSendPacketCorrectly();
    void testPrepareForPersistentLoop_setsUpClientConnection();
    void testPrepareForPersistentLoop_withRealSocket_updatesPorts();

    // characterization tests
    void testPersistentLoop_exitsOnCloseRequest();
    void testPersistentLoop_processesNoDataAndExits();
    void testPersistentLoop_emitsIdleStatusWhenNoData();
    void testPersistentLoop_exitsImmediatelyOnCloseRequest();
    void testPersistentLoop_exitsOnConnectionBroken();
    void testPersistentLoop_cleansUpOnExit();

    // cleanupAfterPersistentConnectionLoop() tests
    void testCleanupAfterPersistentConnectionLoop_whenClientConnectionIsNull_emitsDisconnected();
};


#endif //PERSISTENTCONNECTIONLOOPTESTS_H
