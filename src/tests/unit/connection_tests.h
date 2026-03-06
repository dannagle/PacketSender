//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef TEST_CONNETION_H
#define TEST_CONNETION_H

#include <QtTest/QTest>

class ConnectionTests : public QObject
{
    Q_OBJECT

public:
    // ConnectionTests();
    // ~ConnectionTests();

private slots:
    void testCreationAndId();
    void testDestructionDoesNotCrash();
    void testMultipleInstancesHaveUniqueIds();
    void testThreadStartsAndStops();
};

#endif //TEST_CONNETION_H
