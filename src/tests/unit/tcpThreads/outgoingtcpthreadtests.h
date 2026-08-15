//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCPTHREADTESTS_H
#define OUTGOINGTCPTHREADTESTS_H
#include <QObject>

#include "../../packet.h"


class OutgoingTcpThreadTests : public QObject
{
    Q_OBJECT

private slots:
    void testConstructor_throwsIfPacketToSendPortIsNotSet();
    void testConstructor_throwsIfPacketToSendAddressIsNotSet();
    void testConstructor_setsPersistentFlagFromPacket_data();
    void testConstructor_setsPersistentFlagFromPacket();
    void testConstructor_packetFlagNotExplicitlySetOutsideOfInitInPacket();

    // getDestinationAddress() Tests
    void testGetDestinationAddress();

    // getDestinationPost() Tests
    void testGetDestinationPort();

    // isValid() tests
    void testIsValid_returnsFalseWhenSendPacketDotToIpIsEmptyString();
    void testIsValid_returnsFalseWhenSendPacketDotPortIsZero();
    void testIsValid_returnsFalseWhenSocketHasNotBeenConnectedToHost();
    void testIsValid_returnsTrueWithValidPacketAndSocket();

    // preparePacket() tests
    void testPreparePacket();

    // buildReplyPacket() tests
    void testBuildReplyPacket_data();
    void testBuildReplyPacket();

    // isValidForSending() tests
    void testIsValidForSending_portIsZero();
    void testIsValidForSending_hexStringIsEmpty();
    void testIsValidForSending_toIPIsEmpty();
    void testIsValidForSending_happyPath();

    // shouldSendReply() tests
    void testShouldSendReply_data();
    void testShouldSendReply();

    // sendReplyIfNeeded() tests
    void testSendReplyIfNeeded_exitsEarlyIfShouldSendReplyReturnsFalse();
    void testSendReplyIfNeeded_sendsPacket_whenResponseHexIsSet();
    void testSendReplyIfNeeded_sendsPacket_commandlineOverrides();
    void testSendReplyIfNeeded_doesNOTSendPacket_whenNoResponse();

    // getSmartResponseData() tests
    void testGetSmartResponseData_data();
    void testGetSmartResponseData();

private:
    inline static const QString DEFAULT_ADDRESS = QStringLiteral("127.0.0.1");
    static constexpr unsigned int DEFAULT_PORT = 9999;

    static Packet createPacketForTest(const QString& address= DEFAULT_ADDRESS, unsigned int port= DEFAULT_PORT);

};


#endif //OUTGOINGTCPTHREADTESTS_H
