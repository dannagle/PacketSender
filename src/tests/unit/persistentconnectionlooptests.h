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
};


#endif //PERSISTENTCONNECTIONLOOPTESTS_H
