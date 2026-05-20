//
// Created by Tomas Gallucci on 4/3/26.
//

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QtTest>

#include "packet.h"
#include "testdoubles/MockSslSocket.h"

class TestUtils
{
public:
    static void debugSpy(const QSignalSpy& spy);
    static Packet createPacketForTest();
    static Packet createIdlePacketForTest();
    static MockSslSocket* createMockSocketForTest();
};


#endif //TESTUTILS_H
