//
// Created by Tomas Gallucci on 4/26/26.
//

#include "basetcpthreadtests.h"

#include <QTcpServer>
#include <QSignalSpy>

#include "realqsslsocket.h"
#include "testdoubles/tcpThreads/basetcpthreadtestdouble.h"
#include "testdoubles/MockSslSocket.h"
#include "utils/testutils.h"

Packet getPacketForTest()
{
    Packet p;

    p.toIP = "some address";
    p.port = 666;
    p.hexString = "AA BB CC DD";

    return p;
}

void BaseTcpThreadTests::testConstructor_throwsWhenSocketIsNull()
{
    QVERIFY_EXCEPTION_THROWN(
        BaseTcpThreadTestDouble thread(nullptr),
        std::invalid_argument
    );
}

void BaseTcpThreadTests::testConstructor_setsSocketParentToThis()
{
    auto *mockSock = new MockSslSocket();
    BaseTcpThreadTestDouble thread(mockSock);

    QCOMPARE(mockSock->parent(), &thread);
}

void BaseTcpThreadTests::testConstructor_QThreadHasNoParentByDefault()
{
    auto *mockSock = new MockSslSocket();
    BaseTcpThreadTestDouble thread(mockSock);

    QCOMPARE(thread.parent(), nullptr);
}

void BaseTcpThreadTests::testConstructor_QThreadParentIsSetWhenPassed()
{
    auto *mockSock = new MockSslSocket();
    QObject testParent;                    // simulate Connection or test fixture

    BaseTcpThreadTestDouble thread(mockSock, &testParent);

    QCOMPARE(thread.parent(), &testParent);
}

void BaseTcpThreadTests::testGetSocket_returnsPassedSocket()
{
    auto *mockSock = new MockSslSocket();
    BaseTcpThreadTestDouble thread(mockSock);

    QCOMPARE(thread.getSocketInterface(), mockSock);
}

void BaseTcpThreadTests::testIsValid_returnsTrueWithValidSocket()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));

    auto *clientSock = new MockSslSocket();
    clientSock->setMockConnected(true);
    clientSock->setIsValid(true);

    clientSock->connectToHost("127.0.0.1", server.serverPort());
    QVERIFY(clientSock->waitForConnected(1000));   // wait until connected

    BaseTcpThreadTestDouble thread(clientSock);
    QCOMPARE(thread.isValid(), true);
}

void BaseTcpThreadTests::testIsValid_returnsFalseWithNullSocket()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    thread.setSocketForTest(nullptr);
    QCOMPARE(thread.isValid(), false);
}

void BaseTcpThreadTests::testIsValid_returnsFalseForFreshUnconnectedSocket()
{
    // A freshly created QSslSocket is not valid until bind() or connectToHost() succeeds
    // We do neither in the BaseTcpThreadTestDouble constructor
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    QCOMPARE(thread.isValid(), false);
}

void BaseTcpThreadTests::testIsConnected_data()
{
    QTest::addColumn<QAbstractSocket::SocketState>("socketState");
    QTest::addColumn<bool>("expectedReturnValue");

    QTest::newRow("connected")  << QAbstractSocket::SocketState::ConnectedState << true;
    QTest::newRow("not connected")  << QAbstractSocket::SocketState::UnconnectedState << false;

}

void BaseTcpThreadTests::testIsConnected()
{
    QFETCH(QAbstractSocket::SocketState, socketState);
    QFETCH(bool, expectedReturnValue);

    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(socketState);

    const BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.isConnected(), expectedReturnValue);
}

void BaseTcpThreadTests::testIsConnected_socketInterfaceIsNullPtr()
{
    BaseTcpThreadTestDouble thread(TestUtils::createMockSocketForTest());
    thread.setSocketForTest(nullptr);
    QCOMPARE(thread.isConnected(), false);
}

void BaseTcpThreadTests::testIsPersistent_isFalseInBaseTcpThread()
{
    const auto thread = BaseTcpThread(TestUtils::createMockSocketForTest());
    QCOMPARE(thread.isPersistent(), false);
}

void BaseTcpThreadTests::testIsSocketEncrypted_returnsFalseWhenNotEncrypted()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockEncrypted(false);

    BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.isSocketEncrypted(), false);
}

void BaseTcpThreadTests::testIsSocketEncrypted_returnsSocketState()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockEncrypted(true);

    BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.isSocketEncrypted(), true);
}

void BaseTcpThreadTests::testIsSocketEncrypted_returnsFalseWithNullSocket()
{
    // Constructor throws on null, so we use a test helper or create a valid thread first
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));

    thread.setSocketForTest(nullptr);
    QCOMPARE(thread.isSocketEncrypted(), false);
}

void BaseTcpThreadTests::testGetPeerPort_returnsCorrectValue()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    quint16 expectedPort = server.serverPort();

    auto *clientSock = new RealQSslSocket(new QSslSocket());
    clientSock->connectToHost("127.0.0.1", expectedPort);
    QVERIFY(clientSock->waitForConnected(2000));

    BaseTcpThreadTestDouble thread(clientSock);
    QCOMPARE(thread.getPeerPort(), expectedPort);
}

void BaseTcpThreadTests::testGetPeerPort_returnsOWhenSocketIsNull()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));

    thread.setSocketForTest(nullptr);
    QCOMPARE(thread.getPeerPort(), 0);
}

void BaseTcpThreadTests::testGetLocalPort_returnsCorrectValue()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    quint16 serverPort = server.serverPort();

    auto *clientSock = new RealQSslSocket(new QSslSocket());
    clientSock->connectToHost("127.0.0.1", serverPort);
    QVERIFY(clientSock->waitForConnected(2000));

    BaseTcpThreadTestDouble thread(clientSock);
    quint16 clientLocalPort = thread.getLocalPort();
    QVERIFY2(clientLocalPort > 0 && clientLocalPort != serverPort,
             qPrintable(QString("Client local port (%1) should be a valid ephemeral port "
                                "different from the server listening port (%2)")
                            .arg(clientLocalPort).arg(serverPort)));
}

void BaseTcpThreadTests::testGetLocalPort_returnsOWhenSocketIsNull()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    thread.setSocketForTest(nullptr);
    QCOMPARE(thread.getLocalPort(), 0);
}

// getIPConnectionProtocol() tests
void BaseTcpThreadTests::testGetIPConnectionProtocol_returnsIPv4WhenSocketIsNull()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    thread.setSocketForTest(nullptr);
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

void BaseTcpThreadTests::testGetIPConnectionProtocol_returnsIPv4WhenPeerAddressIsNull()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress());  // null address

    BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

void BaseTcpThreadTests::testGetIPConnectionProtocol_returnsIPv4ForIPv4Peer()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress("192.168.1.100"));

    BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv4Protocol);
}

void BaseTcpThreadTests::testGetIPConnectionProtocol_returnsIPv6ForIPv6Peer()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress("::1"));

    BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.getIPConnectionProtocol(), QAbstractSocket::IPv6Protocol);
}

// getPeerAddressAsString() tests
void BaseTcpThreadTests::testGetPeerAddressAsString_returnsEmptyStringWhenSocketIsNull()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    thread.setSocketForTest(nullptr);
    QVERIFY(thread.getPeerAddressAsString().isEmpty());
}

void BaseTcpThreadTests::testGetPeerAddressAsString_returnsEmptyStringWhenSocketPeerAddressIsNull()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress());

    BaseTcpThreadTestDouble thread(mockSock);
    QVERIFY(thread.getPeerAddressAsString().isEmpty());
}

void BaseTcpThreadTests::testGetPeerAddressAsString_returnsIPV6()
{
    const QString address = QString("::1");
    auto *mockSock = new MockSslSocket();
    mockSock->setMockPeerAddress(QHostAddress(address));

    BaseTcpThreadTestDouble thread(mockSock);
    QCOMPARE(thread.getPeerAddressAsString(), address);
}

// isValidForSending() tests
void BaseTcpThreadTests::testIsValidForSending_portIsZero()
{
    BaseTcpThreadTestDouble thread(TestUtils::createMockSocketForTest());

    Packet p;
    p.toIP = "";
    p.port = 0;
    p.hexString = "AA BB CC DD";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QCOMPARE(returnValue, false);
    QCOMPARE(errorMessage, "Port must be a positive number");
}

void BaseTcpThreadTests::testIsValidForSending_hexStringIsEmpty()
{
    BaseTcpThreadTestDouble thread(TestUtils::createMockSocketForTest());

    Packet p;
    p.toIP = "";
    p.port = 9999;
    p.hexString = "";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QCOMPARE(returnValue, false);
    QCOMPARE(errorMessage, "No data to send (hexString is empty)");
}

void BaseTcpThreadTests::testIsValidForSending_happyPath()
{
    BaseTcpThreadTestDouble thread(TestUtils::createMockSocketForTest());

    Packet p;
    p.toIP = "";
    p.port = 9999;
    p.hexString = "AA BB CC DD";

    QString errorMessage;
    bool returnValue = thread.callIsValidForSending(p, &errorMessage);
    QCOMPARE(returnValue, true);
    QVERIFY(errorMessage.isEmpty());
}

// testSendOutgoingPacket() tests
void BaseTcpThreadTests::testSendOutgoingPacket_socketIsNullptr_emitsConnectionStatusError()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    thread.setSocketForTest(nullptr);

    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callSendOutgoingPacket(*(new Packet()));
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().at(0).toString(),
             QString("Error: No socket available"));
}

void BaseTcpThreadTests::testSendOutgoingPacket_socketIsNullptr_emitsErrorSignalWithSocketAccessError()
{
    BaseTcpThreadTestDouble thread(new RealQSslSocket(new QSslSocket()));
    thread.setSocketForTest(nullptr);

    QSignalSpy errorSignalSpy(&thread, &BaseTcpThread::error);

    thread.callSendOutgoingPacket(*(new Packet()));
    QCOMPARE(errorSignalSpy.count(), 1);
    QCOMPARE(errorSignalSpy.first().at(0).value<QSslSocket::SocketError>(),
         QAbstractSocket::SocketAccessError);
}

void BaseTcpThreadTests::testSendOutgoingPacket_socketIsNotInConnectedState_emitsConnectionStatusError()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::UnconnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    thread.callSendOutgoingPacket(*(new Packet()));
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().at(0).toString(),
             QString("Error: Socket not connected"));
}

void BaseTcpThreadTests::testSendOutgoingPacket_socketIsNotInConnectedState_emitsErrorSignalWithSocketAccessError()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::UnconnectedState);

    BaseTcpThreadTestDouble thread(mockSock);

    QSignalSpy errorSpy(&thread, &BaseTcpThread::error);

    thread.callSendOutgoingPacket(*(new Packet()));
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.first().at(0).value<QSslSocket::SocketError>(),
         QAbstractSocket::SocketAccessError);
}

void BaseTcpThreadTests::testSendOutgoingPacket_packetToIpEmpty_doesNOTemitErrorMessage_DestinationAddressToIpIsEmpty()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::ConnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    Packet p = getPacketForTest();
    p.toIP = "";

    thread.callSendOutgoingPacket(p);
    QCOMPARE(errorMessageSpy.count(), 0);
}

void BaseTcpThreadTests::testSendOutgoingPacket_packetPortIsZero_emitsErrorMessage_PortMustBeAPositiveNumber()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::ConnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    Packet p = getPacketForTest();
    p.port = 0;

    thread.callSendOutgoingPacket(p);

    QCOMPARE(errorMessageSpy.count(), 1);
    QCOMPARE(errorMessageSpy.first().at(0).toString(),
             QString("Port must be a positive number"));
}

void BaseTcpThreadTests::testSendOutgoingPacket_packetHasNoData_emitsErrorMessage_NoDataToSend()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::ConnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy errorMessageSpy(&thread, &BaseTcpThread::errorMessage);

    Packet p = getPacketForTest();
    p.hexString = "";

    thread.callSendOutgoingPacket(p);

    QCOMPARE(errorMessageSpy.count(), 1);
    QCOMPARE(errorMessageSpy.first().at(0).toString(),
             QString("No data to send (hexString is empty)"));
}

void BaseTcpThreadTests::testSendOutgoingPacket_packetHasData_emitsConnectionStatusSendingData()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::ConnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);

    Packet p = getPacketForTest();
    thread.callSendOutgoingPacket(p);
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().at(0).toString(),
         QString("Sending data: ") + p.asciiString());

}

void BaseTcpThreadTests::testSendOutgoingPacket_packetHasData_emitsPacketSent()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::ConnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy packetSentSpy(&thread, &BaseTcpThread::packetSent);

    Packet p = getPacketForTest();
    thread.callSendOutgoingPacket(p);
    QCOMPARE(packetSentSpy.count(), 1);

    Packet emittedPacket = packetSentSpy.first().first().value<Packet>();
    QVERIFY(emittedPacket == p);
}

// closeConnection() tests
void BaseTcpThreadTests::testCloseConnection_SocketIsConnected_DisconnectFromHostCalled()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ConnectedState);

    BaseTcpThreadTestDouble thread(mockSock);

    thread.callCloseConnection();
    QVERIFY(mockSock != nullptr);
    QCOMPARE(mockSock->disconnectFromHostCallCount, 1);
}

void BaseTcpThreadTests::testCloseConnection_SocketIsClosing_DisconnectFromHostCalled()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true);
    mockSock->setMockState(QAbstractSocket::ClosingState);

    BaseTcpThreadTestDouble thread(mockSock);

    thread.callCloseConnection();
    QCOMPARE(mockSock->disconnectFromHostCallCount, 1);
}

void BaseTcpThreadTests::testCloseConnection_CloseCalled()
{
    auto *mockSock = new MockSslSocket();
    BaseTcpThreadTestDouble thread(mockSock);
    thread.callCloseConnection();
    QCOMPARE(mockSock->closeCallCount, 1);
}

void BaseTcpThreadTests::testCloseConnection_emitsConnectionStatusDisconnected_whenSocketIsInConnectedState()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockConnected(true); // sets state to QAbstractSocket::SocketState::Connected

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);
    QCOMPARE(connectionStatusSpy.count(), 0);

    thread.callCloseConnection();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().first().value<QString>(), "Disconnected");
}

void BaseTcpThreadTests::testCloseConnection_emitsConnectionStatusDisconnected_whenSocketIsInClosingState()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::ClosingState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);
    QCOMPARE(connectionStatusSpy.count(), 0);

    thread.callCloseConnection();
    QCOMPARE(connectionStatusSpy.count(), 1);
    QCOMPARE(connectionStatusSpy.first().first().value<QString>(), "Disconnected");
}

void BaseTcpThreadTests::testCloseConnection_doesNotEmitConnectionStatusDisconnected_whenSocketIsNotClosingOrConnectedState()
{
    auto *mockSock = new MockSslSocket();
    mockSock->setMockState(QAbstractSocket::SocketState::UnconnectedState);

    BaseTcpThreadTestDouble thread(mockSock);
    QSignalSpy connectionStatusSpy(&thread, &BaseTcpThread::connectionStatus);
    QCOMPARE(connectionStatusSpy.count(), 0);

    thread.callCloseConnection();
    QCOMPARE(connectionStatusSpy.count(), 0);
}
