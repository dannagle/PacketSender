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

    static bool qStringVectorStartsWith(const std::vector<QString>& vec, const std::vector<QString>& prefix);
    static bool qStringVectorEndsWith(const std::vector<QString>& vec, const std::vector<QString>& suffix);
};


#endif //TESTUTILS_H
