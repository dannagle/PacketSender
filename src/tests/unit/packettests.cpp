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

// operator==() tests
void PacketTests::testOperatorEquals_returnsTrueForIdenticalPackets()
{
    Packet a;
    a.name = "Test Packet";
    a.hexString = "AA BB CC DD";
    a.requestPath = "/api/test";
    a.fromIP = "192.168.1.100";
    a.toIP = "127.0.0.1";
    a.resolvedIP = "127.0.0.1";
    a.errorString = "No error";
    a.repeat = 1.0f;
    a.port = 12345;
    a.fromPort = 54321;
    a.tcpOrUdp = "TCP";
    a.sendResponse = 1;
    a.incoming = false;
    a.receiveBeforeSend = false;
    a.delayAfterConnect = 0;
    a.persistent = true;

    Packet b = a;
    QVERIFY(a == b);
}

void PacketTests::testOperatorEquals_ignoresTimestamp()
{
    Packet a;
    a.hexString = "AA BB CC";
    a.toIP = "127.0.0.1";
    a.port = 12345;

    Packet b = a;
    b.timestamp = QDateTime::currentDateTime().addDays(10); // different timestamp

    QVERIFY(a == b);   // should still be equal
    QVERIFY(!(a != b));
}

void PacketTests::testOperatorEquals_returnsFalseWhenAnyFieldDiffers_data()
{
    QTest::addColumn<QString>("fieldName");
    QTest::addColumn<Packet>("modifiedPacket");

    Packet base;
    base.name = "Base Packet";
    base.hexString = "AA BB CC DD";
    base.requestPath = "/api/test";
    base.fromIP = "192.168.1.100";
    base.toIP = "127.0.0.1";
    base.resolvedIP = "127.0.0.1";
    base.errorString = "No error";
    base.repeat = 1.0f;
    base.port = 12345;
    base.fromPort = 54321;
    base.tcpOrUdp = "TCP";
    base.sendResponse = 1;
    base.incoming = false;
    base.receiveBeforeSend = false;
    base.delayAfterConnect = 0;
    base.persistent = false;

    auto addCase = [&](const QString& name, Packet p) {
        QTest::newRow(name.toUtf8().constData()) << name << p;
    };

    {
        Packet p = base; p.name = "Different Name"; addCase("name", p);
    }
    {
        Packet p = base; p.hexString = "XX YY ZZ"; addCase("hexString", p);
    }
    {
        Packet p = base; p.requestPath = "/api/other"; addCase("requestPath", p);
    }
    {
        Packet p = base; p.fromIP = "10.0.0.1"; addCase("fromIP", p);
    }
    {
        Packet p = base; p.toIP = "192.168.1.1"; addCase("toIP", p);
    }
    {
        Packet p = base; p.resolvedIP = "10.0.0.1"; addCase("resolvedIP", p);
    }
    {
        Packet p = base; p.errorString = "Some error"; addCase("errorString", p);
    }
    {
        Packet p = base; p.repeat = 5.0f; addCase("repeat", p);
    }
    {
        Packet p = base; p.port = 9999; addCase("port", p);
    }
    {
        Packet p = base; p.fromPort = 11111; addCase("fromPort", p);
    }
    {
        Packet p = base; p.tcpOrUdp = "UDP"; addCase("tcpOrUdp", p);
    }
    {
        Packet p = base; p.sendResponse = 0; addCase("sendResponse", p);
    }
    {
        Packet p = base; p.incoming = true; addCase("incoming", p);
    }
    {
        Packet p = base; p.receiveBeforeSend = true; addCase("receiveBeforeSend", p);
    }
    {
        Packet p = base; p.delayAfterConnect = 1000; addCase("delayAfterConnect", p);
    }
    {
        Packet p = base; p.persistent = true; addCase("persistent", p);
    }
}

void PacketTests::testOperatorEquals_returnsFalseWhenAnyFieldDiffers()
{
    QFETCH(QString, fieldName);
    QFETCH(Packet, modifiedPacket);

    Packet base;
    base.name = "Base Packet";
    base.hexString = "AA BB CC DD";
    base.requestPath = "/api/test";
    base.fromIP = "192.168.1.100";
    base.toIP = "127.0.0.1";
    base.resolvedIP = "127.0.0.1";
    base.errorString = "No error";
    base.repeat = 1.0f;
    base.port = 12345;
    base.fromPort = 54321;
    base.tcpOrUdp = "TCP";
    base.sendResponse = 1;
    base.incoming = false;
    base.receiveBeforeSend = false;
    base.delayAfterConnect = 0;
    base.persistent = false;

    QVERIFY2(!(modifiedPacket == base),
             qPrintable("operator== failed to detect difference in field: " + fieldName));
}


// operator!=() tests
void PacketTests::testOperatorNotEquals_returnsFalseForIdenticalPackets()
{
    Packet a;
    a.name = "Test Packet";
    a.hexString = "AA BB CC DD";
    a.requestPath = "/api/test";
    a.fromIP = "192.168.1.100";
    a.toIP = "127.0.0.1";
    a.resolvedIP = "127.0.0.1";
    a.errorString = "No error";
    a.repeat = 1.0f;
    a.port = 12345;
    a.fromPort = 54321;
    a.tcpOrUdp = "TCP";
    a.sendResponse = 1;
    a.incoming = false;
    a.receiveBeforeSend = false;
    a.delayAfterConnect = 0;
    a.persistent = true;

    Packet b = a;
    QVERIFY(!(a != b));
}

void PacketTests::testOperatorNotEquals_returnsTrueWhenAnyFieldDiffers_data()
{
    QTest::addColumn<QString>("fieldName");
    QTest::addColumn<Packet>("modifiedPacket");

    Packet base;
    base.name = "Base Packet";
    base.hexString = "AA BB CC DD";
    base.requestPath = "/api/test";
    base.fromIP = "192.168.1.100";
    base.toIP = "127.0.0.1";
    base.resolvedIP = "127.0.0.1";
    base.errorString = "No error";
    base.repeat = 1.0f;
    base.port = 12345;
    base.fromPort = 54321;
    base.tcpOrUdp = "TCP";
    base.sendResponse = 1;
    base.incoming = false;
    base.receiveBeforeSend = false;
    base.delayAfterConnect = 0;
    base.persistent = false;

    auto addCase = [&](const QString& name, Packet p) {
        QTest::newRow(name.toUtf8().constData()) << name << p;
    };

    {
        Packet p = base; p.name = "Different Name"; addCase("name", p);
    }
    {
        Packet p = base; p.hexString = "XX YY ZZ"; addCase("hexString", p);
    }
    {
        Packet p = base; p.requestPath = "/api/other"; addCase("requestPath", p);
    }
    {
        Packet p = base; p.fromIP = "10.0.0.1"; addCase("fromIP", p);
    }
    {
        Packet p = base; p.toIP = "192.168.1.1"; addCase("toIP", p);
    }
    {
        Packet p = base; p.resolvedIP = "10.0.0.1"; addCase("resolvedIP", p);
    }
    {
        Packet p = base; p.errorString = "Some error"; addCase("errorString", p);
    }
    {
        Packet p = base; p.repeat = 5.0f; addCase("repeat", p);
    }
    {
        Packet p = base; p.port = 9999; addCase("port", p);
    }
    {
        Packet p = base; p.fromPort = 11111; addCase("fromPort", p);
    }
    {
        Packet p = base; p.tcpOrUdp = "UDP"; addCase("tcpOrUdp", p);
    }
    {
        Packet p = base; p.sendResponse = 0; addCase("sendResponse", p);
    }
    {
        Packet p = base; p.incoming = true; addCase("incoming", p);
    }
    {
        Packet p = base; p.receiveBeforeSend = true; addCase("receiveBeforeSend", p);
    }
    {
        Packet p = base; p.delayAfterConnect = 1000; addCase("delayAfterConnect", p);
    }
    {
        Packet p = base; p.persistent = true; addCase("persistent", p);
    }
}

void PacketTests::testOperatorNotEquals_returnsTrueWhenAnyFieldDiffers()
{
    // Reuse the same data-driven cases
    QFETCH(QString, fieldName);
    QFETCH(Packet, modifiedPacket);

    Packet base;
    base.name = "Base Packet";
    base.hexString = "AA BB CC DD";
    base.requestPath = "/api/test";
    base.fromIP = "192.168.1.100";
    base.toIP = "127.0.0.1";
    base.resolvedIP = "127.0.0.1";
    base.errorString = "No error";
    base.repeat = 1.0f;
    base.port = 12345;
    base.fromPort = 54321;
    base.tcpOrUdp = "TCP";
    base.sendResponse = 1;
    base.incoming = false;
    base.receiveBeforeSend = false;
    base.delayAfterConnect = 0;
    base.persistent = false;

    // We already know == is false from the previous test.
    // Verify that != returns the opposite.
    QVERIFY(modifiedPacket != base);
    QVERIFY(!(modifiedPacket == base));
}
