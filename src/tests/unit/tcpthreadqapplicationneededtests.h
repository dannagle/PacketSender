//
// Created by Tomas Gallucci on 3/15/26.
//

#ifndef TCPTHREDQAPPLICATIONNEEDEDTESTS_H
#define TCPTHREDQAPPLICATIONNEEDEDTESTS_H

#include <QObject>

class TcpThread_QApplicationNeeded_tests: public QObject
{
    Q_OBJECT
private slots:
    void testDestructorWaitsGracefullyWhenManaged();
    void testFullLifecycleWithServer();
    void testOutgoingClientPathStartsLoopAndSendsPacket();

    // characterization tests
    void testRunOutgoingConnectFailure();
    void testRunOutgoingCloseDuringLoop();

    // SSL handshake handler tests - INCOMING
    void testHandleIncomingSSLHandshake_success();
    void testHandleIncomingSSLHandshake_withErrors();

    // SSL handshake handler tests - OUTGOING
    void testHandleOutgoingSSLHandshake_success();
    void testHandleOutgoingSSLHandshake_withErrors();

    // runOutgoingClient characterization tests
    void testRunOutgoingClient_plainTCP_connectFailure();
    void testRunOutgoingClient_SSL_path_is_attempted();

};


#endif //TCPTHREDQAPPLICATIONNEEDEDTESTS_H
