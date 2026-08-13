//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef BASETCPCONNECTIONTESTS_H
#define BASETCPCONNECTIONTESTS_H

#include <QObject>

#include "../../tcpThreads/basetcpthread.h"
#include "../testdoubles/basetcpthreadtestdouble.h"

class BaseTcpConnectionTests : public QObject
{
    Q_OBJECT

private slots:
    // isConnected() tests
    void testIsConnected_isNotDeterminedDirectlyBySocketConnectivity_data();
    void testIsConnected_isNotDeterminedDirectlyBySocketConnectivity();

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

    // getClassName() tests
    void testGetClassName();

    // send() tests
    void testSend_throwsRuntimeException();

    // receiveData() tests
    void testReceiveData_throwsRuntimeException();

    // terminate() tests
    void testTerminate_startState_threadIsDisconnected();
    void testTerminate_startState_threadIsNullptr();
    void testTerminate_startState_threadIsConnected();

    // close() tests
    void testClose_callsTerminate();

private:
    std::unique_ptr<BaseTcpThreadTestDouble> createThreadWithConnectionState(QAbstractSocket::SocketState socketState);
};


#endif //BASETCPCONNECTIONTESTS_H
