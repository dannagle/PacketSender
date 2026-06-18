//
// Created by Tomas Gallucci on 6/16/26.
//

#include <qtestcase.h>

#include "connectiontests.h"
#include "connections/connection.h"
#include "../testdoubles/connections/ConnectionTestDouble.h"

void ConnectionTests::testConnectionConstructor_createsConnectionObjectWithID()
{
    const ConnectionTestDouble connection{};

    const QString id = connection.id();
    QUuid uuid(id);
    QVERIFY(uuid.isNull() == false);
    QCOMPARE(uuid.toString(QUuid::WithoutBraces), id);
}

void ConnectionTests::testGetClassName()
{
    const ConnectionTestDouble connection{};
    QCOMPARE(connection.callGetClassName(), "ConnectionTestDouble");
}
