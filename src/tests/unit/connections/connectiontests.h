//
// Created by Tomas Gallucci on 6/16/26.
//

#ifndef CONNECTIONTESTS_H
#define CONNECTIONTESTS_H
#include <QObject>


class ConnectionTests : public QObject
{
    Q_OBJECT
private slots:
    // constructor tests
    void testConnectionConstructor_createsConnectionObjectWithID();

    // getClassName() tests
    void testGetClassName();

    // State tests
    void testDefaultStateIsCreated();
    void testSocketSuccessfullySetsSocketDescriptor_TransitionsStateToActive();
    void testSocketDisconnectedBeforeIncomingDataRead_TransitionsStateToInactive();
    void testIncomingSSL_success_setsConnectionStateToActive();
    void testIncomingSSL_failure_setsConnectionStateToInactive();
    void testHandleOutgoingSSL_success_setsConnectionStateToActive();
    void testHandleOutgoingSSL_failure_setsConnectionStateToInactive();
    void testTerminate_TransitionsStateToClosing_whenThreadIsNullptr();
    void testTerminate_TransitionsStateToClosed_whenThreadIsNOTNullptr();
    void testHandleOutgoingPlainTCP_success_setsConnectionStateToActive();
    void testHandleOutgoingPlainTCP_unsuccessful_setsConnectionStateToInactive();
};


#endif //CONNECTIONTESTS_H
