//
// Created by Tomas Gallucci on 4/3/26.
//

#include <QtTest/QTest.h>
#include <QSignalSpy>

#include "testutils.h"


void TestUtils::debugSpy(const QSignalSpy& spy)
{
    qDebug() << "QSignalSpy captured" << spy.count() << "emissions";
    for (int i = 0; i < spy.count(); ++i) {
        QList<QVariant> args = spy.at(i);
        qDebug() << "  Emission" << i << "has" << args.size() << "arguments:";
        for (int j = 0; j < args.size(); ++j) {
            qDebug() << "    Arg" << j << ":" << args.at(j)
                     << "(type:" << args.at(j).typeName() << ")";
        }
    }
}

Packet TestUtils::createPacketForTest()
{
    Packet p;

    p.toIP = "127.0.0.1";
    p.port = 666;
    p.hexString = "AA BB CC DD";
    p.tcpOrUdp = "TCP";

    return p;
}

MockSslSocket* TestUtils::createMockSocketForTest()
{
    auto mockSock = new MockSslSocket();;

    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    return mockSock;
}
