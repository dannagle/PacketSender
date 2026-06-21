//
// Created by Tomas Gallucci on 6/20/26.
//

#ifndef OUTGOINGTCPCONNECTIONTESTS_H
#define OUTGOINGTCPCONNECTIONTESTS_H
#include <QObject>


class OutgoingTcpConnectionTests : public QObject
{
    Q_OBJECT

private slots:
    void test_send_threadDoesNotExist();
    void test_send_replacesExistingThread();
};



#endif //OUTGOINGTCPCONNECTIONTESTS_H
