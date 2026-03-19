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

    // tryConnectEncrypted() tests
    void testTryConnectEncrypted_success_emitsCipherAndCertInfo();
    void testTryConnectEncrypted_sslErrors_emitsErrorPackets();
    void testTryConnectEncrypted_connectFailure_returnsFalse();

    // clientSocket() tests
    void testClientSocket_lazyCreation_createsRealSocketOnFirstCall();
    void testClientSocket_lazyCreation_returnsExistingSocketOnSecondCall();
    void testInjectionConstructor_assignsMockSocket();
    void characterizeRun_outgoingSsl_connectsAndEmitsCipherPackets();
};


#endif //TCPTHREADTESTS_H
