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
    void testTerminate_TransitionsStateToClosing_whenThreadIsNullptr();
    void testTerminate_TransitionsStateToClosed_whenThreadIsNOTNullptr();
    void testHandleOutgoingPlainTCP_success_setsConnectionStateToActive();
    void testHandleOutgoingPlainTCP_unsuccessful_setsConnectionStateToInactive();
};


#endif //CONNECTIONTESTS_H
