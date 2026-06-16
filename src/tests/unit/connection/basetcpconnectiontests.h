//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef CONNECTIONTESTS_H
#define CONNECTIONTESTS_H

#include <QObject>

#include "basetcpthread.h"
#include "../testdoubles/basetcpthreadtestdouble.h"

class BaseTcpConnectionTests : public QObject
{
    Q_OBJECT

private slots:
    // constructor tests
    void testConnectionConstructor_createsConnectionObjectWithID();

    // isConnected() tests
    void testIsConnected_data();
    void testIsConnected();

    // isSecure() tests
    void testIsSecure_data();
    void testIsSecure();

    // isIncoming() tests
    void testIsIncoming_returnsTrue_whenThreadIsIncomingTcpThread();
    void testIsIncoming_returnsFalse_whenThreadIsOutgoingTcpThread();

    // isPersistent() tests
    void testIsPersistent_Incoming_data();
    void testIsPersistent_Incoming();
    void testIsPersistent_Outgoing_data();
    void testIsPersistent_Outgoing();
    // void testIsPersistent_data();
    // void testIsPersistent();

private:
    std::unique_ptr<BaseTcpThreadTestDouble> createThreadWithConnectionState(QAbstractSocket::SocketState socketState);
};


#endif //CONNECTIONTESTS_H
