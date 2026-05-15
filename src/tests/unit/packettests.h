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

    // operator==() tests
    void testOperatorEquals_returnsTrueForIdenticalPackets();
    void testOperatorEquals_ignoresTimestamp();

    void testOperatorEquals_returnsFalseWhenAnyFieldDiffers_data(); // QtTest's version of parameterized testing
    void testOperatorEquals_returnsFalseWhenAnyFieldDiffers();

    // operator!=() tests
    void testOperatorNotEquals_returnsFalseForIdenticalPackets();
    void testOperatorNotEquals_returnsTrueWhenAnyFieldDiffers_data(); // QtTest's version of parameterized testing
    void testOperatorNotEquals_returnsTrueWhenAnyFieldDiffers();
};


#endif //PACKETTESTS_H
