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
    // createIncomingTcpConnection() tests
    void testCreateIncomingTcpConnection_returnsPair();
    void testCreateIncomingTcpConnection_callsSetupConnectionSignals();

    // createOutgoingTcpConnection() tests
    void testCreateOutgoingTcpConnection_returnsPair();
    void testCreateOutgoingTcpConnection_callsSetupConnectionSignals();

    // create*() tests
    void testCreateMultipleConnections_idIncreasesMonotonically();

    // close() tests
    void testClose();

    // shutdownAll() tests
    void testShutdownAll();
};


#endif //CONNECTIONMANAGERTESTS_H
