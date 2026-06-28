//
// Created by Tomas Gallucci on 3/6/26.
//

#ifndef CONNECTIONMANAGERTESTS_H
#define CONNECTIONMANAGERTESTS_H


#include <QtTest/QTest>
#include "connectionmanager.h"


class ConnectionManagerTests : public QObject
{
    Q_OBJECT


private slots:
//     void init();
//     void cleanup();
//
    // createIncomingTcpConnection() tests
    void testCreateIncomingTcpConnection_returnsPair();

    // createOutgoingTcpConnection() tests
    void testCreateOutgoingTcpConnection_returnsPair();

    // create*() tests
    void testCreateMultipleConnections_idIncreasesMonotonically();
    void testClose();

    //     void testCloseRemovesConnection();
//     void testShutdownAllClearsAllConnections();
};


#endif //CONNECTIONMANAGERTESTS_H
