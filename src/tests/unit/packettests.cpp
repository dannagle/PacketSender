//
// Created by Tomas Gallucci on 5/14/26.
//

#include "packettests.h"
#include "../../packet.h"

void PacketTests::test_isValidForSending_toIPIsEmpty_returnsFalse()
{
    Packet p;
    p.toIP = "";
    QCOMPARE(false, p.isValidForSending());
}

void PacketTests::test_isValidForSending_toIPIsEmpty_outParameterValue()
{
    Packet p;
    p.toIP = "";

    QString errorMessage = "";
    p.isValidForSending(&errorMessage);

    const QString expected = "Destination address (toIP) is empty";
    QCOMPARE(errorMessage, expected);
}

void PacketTests::test_isValidForSending_portIsZero_returnsFalse()
{
    Packet p;
    p.toIP = "has some value";
    p.port = 0;
    QCOMPARE(false, p.isValidForSending());
}

void PacketTests::test_isValidForSending_portIsZero_outParameterValue()
{
    Packet p;
    p.toIP = "has some value";
    p.port = 0;

    QString errorMessage = "";
    p.isValidForSending(&errorMessage);

    const QString expected = "Port must be a positive number";
    QCOMPARE(errorMessage, expected);
}

void PacketTests::test_isValidForSending_byteArrayIsEmpty_returnsFalse()
{
    Packet p;
    p.toIP = "has some value";
    p.port = 666;
    p.hexString = "";
    QCOMPARE(false, p.isValidForSending());

}

void PacketTests::test_isValidForSending_byteArrayIsEmpty_outParameterValue()
{
    Packet p;
    p.toIP = "has some value";
    p.port = 666;
    p.hexString = "";

    QString errorMessage = "";
    p.isValidForSending(&errorMessage);

    const QString expected = "No data to send (hexString is empty)";
    QCOMPARE(errorMessage, expected);
}

void PacketTests::test_isValidForSending_happyPath_returnsTrue()
{
    Packet p;
    p.toIP = "has some value";
    p.port = 666;
    p.hexString = "foo bar baz";
    QVERIFY(p.isValidForSending());
}

void PacketTests::test_isValidForSending_happyPath_outParameterValue()
{
    Packet p;
    p.toIP = "has some value";
    p.port = 666;
    p.hexString = "foo bar baz";

    QString errorMessage = "";
    p.isValidForSending(&errorMessage);

    QVERIFY(errorMessage.isEmpty());
}
