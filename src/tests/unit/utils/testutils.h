//
// Created by Tomas Gallucci on 4/3/26.
//

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QtTest>

#include "packet.h"
#include "../testdoubles/MockSslSocket.h"

class TestUtils
{
public:
    static void debugSpy(const QSignalSpy& spy);
    static Packet createPacketForTest();
    static Packet createIdlePacketForTest();
    static MockSslSocket* createMockSocketForTest();
    static QString extractResourceToTempFile(const QString& resourcePath);
    static void setupProductionSnakeOilCertsForTest();
};


#endif //TESTUTILS_H
