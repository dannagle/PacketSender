//
// Created by Tomas Gallucci on 6/17/26.
//

#include "incomingtcpconnectiontests.h"

#include "incomingtcpthread.h"
#include "connections/incomingtcpconnection.h"
#include "testdoubles/connections/incomingtcpconnectiontestdouble.h"
#include "tests/unit/utils/testutils.h"

void IncomingTcpConnectionTests::testIsIncoming()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketForTest());
    auto conn = std::make_unique<IncomingTcpConnectionTestDouble>();
    conn->setThread(std::move(thread));
    QCOMPARE(conn->isIncoming(), true);
}

void IncomingTcpConnectionTests::testSend_throwsRuntimeException()
{
    auto thread = std::make_unique<IncomingTcpThread>(TestUtils::createMockSocketForTest());
    auto connection = std::make_unique<IncomingTcpConnectionTestDouble>();

    const auto p = TestUtils::createPacketForTest();

    connection->setThread(std::move(thread));

    try
    {
        connection->send(p);
        QFAIL("send should have thrown");
    } catch(std::runtime_error& e)
    {
        // Because we're using a TestDouble, the error message will read
        // "Unsupported Operation: IncomingTcpConnectionTestDouble cannot send Packet"
        // but production code will read "Unsupported Operation: IncomingTcpConnection\\w* cannot send Packet"
        QRegularExpression re("Unsupported Operation: IncomingTcpConnection\\w* cannot send Packet");
        auto fromString = QString::fromStdString(e.what());
        QVERIFY(re.match(fromString).hasMatch());
    }
}
