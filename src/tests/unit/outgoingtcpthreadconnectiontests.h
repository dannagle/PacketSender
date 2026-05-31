//
// Created by Tomas Gallucci on 5/30/26.
//

#ifndef OUTGOINGTCPTHREADCONNECTIONTESTS_H
#define OUTGOINGTCPTHREADCONNECTIONTESTS_H
#include <QObject>


class OutgoingTcpThreadConnectionTests : public QObject
{
    Q_OBJECT

private slots:
    // handleOutgoingPlainTCP tests()
    void testHandleOutgoingPlainTCP_callsConnectToHost();
    void testHandleOutgoingPlainTCP_emitsSuccess();
    void testHandleOutgoingPlainTCP_callsHandleConnectionFailure();

    // handleConnectionFailure() tests
    void testHandleConnectionFailure_emitsConnectionStatus_CouldNotConnect();
    void testHandleConnectionFailure_emitsErrorMessage();
    void testHandleConnectionFailure_emitsPacketSent();
};


#endif //OUTGOINGTCPTHREADCONNECTIONTESTS_H
