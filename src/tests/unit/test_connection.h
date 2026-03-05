//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef TEST_CONNETION_H
#define TEST_CONNETION_H

#include <QtTest/QTest>

class TestConnection : public QObject
{
    Q_OBJECT

public:
    // TestConnection();
    // ~TestConnection();

private slots:
    void testCreationAndId();
    void testDestructionDoesNotCrash();
    void testMultipleInstancesHaveUniqueIds();
};

#endif //TEST_CONNETION_H
