//
// Created by Tomas Gallucci on 6/16/26.
//

#include <QUuid>
#include <qtestcase.h>

#include "connectiontests.h"
#include "connections/connection.h"
#include "testdoubles/ConnectionTestDouble.h"

void ConnectionTests::testConnectionConstructor_createsConnectionObjectWithID()
{
    ConnectionTestDouble connection{};

    const QString id = connection.id();
    QUuid uuid(id);
    QVERIFY(uuid.isNull() == false);
    QCOMPARE(uuid.toString(QUuid::WithoutBraces), id);
}
