//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCPTHREADTESTS_H
#define OUTGOINGTCPTHREADTESTS_H
#include <QObject>

#include "packet.h"


class OutgoingTcpTests : public QObject
{
    Q_OBJECT

private slots:
    void testConstructor_throwsIfPacketToSendPortIsNotSet();
    void testConstructor_throwsIfPacketToSendAddressIsNotSet();

    void testGetDestinationAddress();
    void testGetDestinationPort();

    void testIsValid_returnsFalseWhenSendPacketDotToIpIsEmptyString();
    void testIsValid_returnsFalseWhenSendPacketDotPortIsZero();
    void testIsValid_returnsFalseWhenSocketHasNotBeenConnectedToHost();
    void testIsValid_returnsTrueWithValidPacketAndSocket();

private:
    inline static const QString DEFAULT_ADDRESS = QStringLiteral("127.0.0.1");
    static constexpr unsigned int DEFAULT_PORT = 9999;

    static Packet createPacketForTest(const QString& address= DEFAULT_ADDRESS, unsigned int port= DEFAULT_PORT);

};


#endif //OUTGOINGTCPTHREADTESTS_H
