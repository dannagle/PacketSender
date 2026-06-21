//
// Created by Tomas Gallucci on 6/17/26.
//

#ifndef INCOMINGTCPCONNECTIONTESTS_H
#define INCOMINGTCPCONNECTIONTESTS_H
#include<QObject>


class IncomingTcpConnectionTests : public QObject
{
    Q_OBJECT

private slots:
    // isIncoming() tests
    void testIsIncoming();

    // send() tests
    void testSend_throwsRuntimeException();

};



#endif //INCOMINGTCPCONNECTIONTESTS_H
