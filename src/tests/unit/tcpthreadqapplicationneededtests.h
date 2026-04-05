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
};


#endif //TCPTHREDQAPPLICATIONNEEDEDTESTS_H
