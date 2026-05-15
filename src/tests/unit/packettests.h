//
// Created by Tomas Gallucci on 5/14/26.
//

#ifndef PACKETTESTS_H
#define PACKETTESTS_H

#include <QtTest/QtTest>

class PacketTests: public QObject
{
    Q_OBJECT

private slots:
    void test_isValidForSending_toIPIsEmpty_returnsFalse();
    void test_isValidForSending_toIPIsEmpty_outParameterValue();

    void test_isValidForSending_portIsZero_returnsFalse();
    void test_isValidForSending_portIsZero_outParameterValue();

    void test_isValidForSending_byteArrayIsEmpty_returnsFalse();
    void test_isValidForSending_byteArrayIsEmpty_outParameterValue();

    void test_isValidForSending_happyPath_returnsTrue();
    void test_isValidForSending_happyPath_outParameterValue();
};


#endif //PACKETTESTS_H
