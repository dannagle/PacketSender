//
// Created by Tomas Gallucci on 6/16/26.
//

#ifndef CONNECTIONTESTS_H
#define CONNECTIONTESTS_H
#include <QObject>


class ConnectionTests : public QObject
{
private slots:
    void testConnectionConstructor_createsConnectionObjectWithID();
};


#endif //CONNECTIONTESTS_H
