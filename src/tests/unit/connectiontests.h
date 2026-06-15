//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef CONNECTIONTESTS_H
#define CONNECTIONTESTS_H

#include <QObject>

#include "basetcpthread.h"
#include "testdoubles/basetcpthreadtestdouble.h"
#include "testdoubles/MockSslSocket.h"

class ConnectionTests : public QObject
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

private:
    std::unique_ptr<BaseTcpThreadTestDouble> createThreadWithConnectionState(QAbstractSocket::SocketState socketState);
};


#endif //CONNECTIONTESTS_H
