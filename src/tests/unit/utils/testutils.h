//
// Created by Tomas Gallucci on 4/3/26.
//

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QtTest>
#include <QTcpServer>

#include "packet.h"
#include "../testdoubles/MockSslSocket.h"
#include "connections/connection.h"

class TestUtils
{
public:
    static void debugSpy(const QSignalSpy& spy);
    static Packet createPacketForTest();
    static Packet createIdlePacketForTest();
    static MockSslSocket* createMockSocketForTest();
    static MockSslSocket* createMockSocketWithSocketDescriptorOtherThan0();
    static QString extractResourceToTempFile(const QString& resourcePath);
    static void setupProductionSnakeOilCertsForTest();

    static bool qStringVectorStartsWith(const std::vector<QString>& vec, const std::vector<QString>& prefix);
    static bool qStringVectorEndsWith(const std::vector<QString>& vec, const std::vector<QString>& suffix);
    static QTcpServer& startQTcpServer();

    // Returns true if any signal in the spy matches the predicate
    static bool signalSpyContains(QSignalSpy& spy,
                           const std::function<bool(const QList<QVariant>&)>& predicate);

    // Convenience overloads
    static bool signalSpyContainsMessage(QSignalSpy& spy, const QString& message);
    static bool signalSpyContainsMessageStartsWith(QSignalSpy& spy, const QString& prefix);
    static bool signalSpyContainsMessageEndsWith(QSignalSpy& spy, const QString& suffix);

    // Count helpers
    static int signalSpyCountMessage(QSignalSpy& spy, const QString& message);
    static int signalSpyCountMessageStartsWith(QSignalSpy& spy, const QString& prefix);
    static int signalSpyCountMessageEndsWith(QSignalSpy& spy, const QString& suffix);
    static constexpr std::string_view toString(Connection::State state);
};


#endif //TESTUTILS_H
