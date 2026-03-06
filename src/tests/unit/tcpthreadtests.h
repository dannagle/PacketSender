//
// Created by Tomas Gallucci on 3/6/26.
//

#ifndef TCPTHREADTESTS_H
#define TCPTHREADTESTS_H

#include <QObject>


class TcpThreadTests : public QObject
{
    Q_OBJECT
private slots:
    void testIncomingConstructorBasic();
    void testIncomingConstructorWithSecureFlag();
};


#endif //TCPTHREADTESTS_H
