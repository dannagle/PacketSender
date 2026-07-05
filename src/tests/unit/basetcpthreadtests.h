//
// Created by Tomas Gallucci on 4/26/26.
//

#ifndef BASETCPTHREADTESTS_H
#define BASETCPTHREADTESTS_H


#include <QtTest/QtTest>

class BaseTcpThreadTests : public QObject
{
    Q_OBJECT

private slots:
    // constructor tests
    void testConstructor_throwsWhenSocketIsNull();
    void testConstructor_setsSocketParentToThis();
    void testConstructor_QThreadHasNoParentByDefault();
    void testConstructor_QThreadParentIsSetWhenPassed();

    // getSocket() test
    void testGetSocket_returnsPassedSocket();

    // isValid() test
    void testIsValid_returnsTrueWithValidSocket();
    void testIsValid_returnsFalseWithNullSocket();
    void testIsValid_returnsFalseForFreshUnconnectedSocket();

    // isConnected() tests
    void testIsConnected_data();
    void testIsConnected();
    void testIsConnected_socketInterfaceIsNullPtr();

    // isPersistent() tests
    void testIsPersistent_isFalseInBaseTcpThread();

    // isSocketEncrypted() tests
    void testIsSocketEncrypted_returnsFalseWhenNotEncrypted();
    void testIsSocketEncrypted_returnsSocketState();
    void testIsSocketEncrypted_returnsFalseWithNullSocket();

    // getPeerPort() tests
    void testGetPeerPort_returnsCorrectValue();
    void testGetPeerPort_returnsOWhenSocketIsNull();

    // getLocalPort() tests
    void testGetLocalPort_returnsCorrectValue();
    void testGetLocalPort_returnsOWhenSocketIsNull();

    // getIPConnectionProtocol() tests
    void testGetIPConnectionProtocol_returnsIPv4WhenSocketIsNull();
    void testGetIPConnectionProtocol_returnsIPv4WhenPeerAddressIsNull();
    void testGetIPConnectionProtocol_returnsIPv4ForIPv4Peer();
    void testGetIPConnectionProtocol_returnsIPv6ForIPv6Peer();

    // getPeerAddressAsString() tests
    void testGetPeerAddressAsString_returnsEmptyStringWhenSocketIsNull();
    void testGetPeerAddressAsString_returnsEmptyStringWhenSocketPeerAddressIsNull();
    void testGetPeerAddressAsString_returnsIPV6();

    // sendOutgoingPacket() tests
    void testSendOutgoingPacket_socketIsNullptr_emitsConnectionStatusError();
    void testSendOutgoingPacket_socketIsNullptr_emitsErrorSignalWithSocketAccessError();

    void testSendOutgoingPacket_socketIsNotInConnectedState_emitsConnectionStatusError();
    void testSendOutgoingPacket_socketIsNotInConnectedState_emitsErrorSignalWithSocketAccessError();

    void testSendOutgoingPacket_packetToIpEmpty_emitsErrorMessage_DestinationAddressToIpIsEmpty();
    void testSendOutgoingPacket_packetPortIsZero_emitsErrorMessage_PortMustBeAPositiveNumber();
    void testSendOutgoingPacket_packetHasNoData_emitsErrorMessage_NoDataToSend();

    void testSendOutgoingPacket_packetHasData_emitsConnectionStatusSendingData();
    void testSendOutgoingPacket_packetHasData_emitsPacketSent();

    // closeConnection() tests
    void testCloseConnection_SocketIsConnected_DisconnectFromHostCalled();
    void testCloseConnection_SocketIsClosing_DisconnectFromHostCalled();
    void testCloseConnection_CloseCalled();
    void testCloseConnection_emitsConnectionStatusDisconnected_whenSocketIsInConnectedState();
    void testCloseConnection_emitsConnectionStatusDisconnected_whenSocketIsInClosingState();
    void testCloseConnection_doesNotEmitConnectionStatusDisconnected_whenSocketIsNotClosingOrConnectedState();
};


#endif //BASETCPTHREADTESTS_H
