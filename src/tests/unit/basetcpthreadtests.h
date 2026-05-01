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
};


#endif //BASETCPTHREADTESTS_H
