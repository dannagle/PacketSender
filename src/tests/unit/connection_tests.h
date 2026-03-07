//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef TEST_CONNETION_H
#define TEST_CONNETION_H

#include <QtTest/QTest>
#include <QElapsedTimer>

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

    void init()
    {
        currentTestTimer.start();
    }

    void cleanup()
    {
        qDebug() << "Test" << QTest::currentTestFunction() << "took" << currentTestTimer.elapsed() << "ms";
    }

private:
    QElapsedTimer currentTestTimer;

};

#endif //TEST_CONNETION_H
