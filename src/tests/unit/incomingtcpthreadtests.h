//
// Created by Tomas Gallucci on 6/8/26.
//

#ifndef INCOMINGTCPTHREADTESTS_H
#define INCOMINGTCPTHREADTESTS_H
#include <QObject>


class IncomingTcpThreadTests : public QObject
{
    Q_OBJECT

private slots:
    // constructor tests
    void testConstructor_assignsSocketDescriptor();
    void testConstructor_assignsIsSecure();

    void testBuildInitialReceivedPacket_socketInterfaceIsNullptr_data();

    // buildInitialReceivedPacket() tests
    void testBuildInitialReceivedPacket_socketInterfaceIsNullptr();
    void testBuildInitialReceivedPacket_socketInterfaceIsNotNullptr();

private:
    static constexpr int TEST_PORT_NUMBER = 666;
};


#endif //INCOMINGTCPTHREADTESTS_H
