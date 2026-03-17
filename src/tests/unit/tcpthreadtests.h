//
// Created by Tomas Gallucci on 3/6/26.
//

#ifndef TCPTHREADTESTS_H
#define TCPTHREADTESTS_H

#include <QObject>


class TcpThreadTests : public QObject
{
    Q_OBJECT
private slots:
    void testIncomingConstructorBasic();
    void testIncomingConstructorWithSecureFlag();

    // getIPConnectionProtocol() tests
    void testGetIPConnectionProtocol_bothIPv4_returnsIPv4();
    void testGetIPConnectionProtocol_bothIPv6_returnsIPv6();
    void testGetIPConnectionProtocol_hostIPv4_packetIPv6_returnsPacketValue();
    void testGetIPConnectionProtocol_hostIPv6_packetIPv4_returnsPacketValue();
    void testGetIPConnectionProtocol_hostMappedIPv4_returnsIPv4();
    void testGetIPConnectionProtocol_hostEmpty_packetIPv4_returnsIPv4();
    void testGetIPConnectionProtocol_hostEmpty_packetIPv6_returnsIPv6();
    void testGetIPConnectionProtocol_bothEmpty_returnsIPv4();
    void testGetIPConnectionProtocol_hostInvalid_packetIPv4_returnsIPv4();
};


#endif //TCPTHREADTESTS_H
