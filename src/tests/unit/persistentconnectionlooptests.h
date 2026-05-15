//
// Created by Tomas Gallucci on 4/11/26.
//

#ifndef PERSISTENTCONNECTIONLOOPTESTS_H
#define PERSISTENTCONNECTIONLOOPTESTS_H

#include <QObject>
#include <QSignalSpy>

class PersistentConnectionLoopTests : public QObject
{
    Q_OBJECT

private slots:
    void testPrepareForPersistentLoop_preparesSendPacketCorrectly();
    void testPrepareForPersistentLoop_setsUpClientConnection();
    void testPrepareForPersistentLoop_withRealSocket_updatesPorts();

    // characterization tests
    void testPersistentLoop_exitsOnCloseRequest();
    void testPersistentLoop_processesNoDataAndExits();
    void testPersistentLoop_emitsIdleStatusWhenNoData();
    void testPersistentLoop_exitsImmediatelyOnCloseRequest();
    void testPersistentLoop_exitsOnConnectionBroken();
    void testPersistentLoop_cleansUpOnExit();

    // cleanupAfterPersistentConnectionLoop() tests
    void testCleanupAfterPersistentConnectionLoop_whenClientConnectionIsNull_emitsDisconnected();
    void testCleanupAfterPersistentConnectionLoop_whenSocketIsConnected_performsFullCleanup();
    void testCleanupAfterPersistentConnectionLoop_whenManagedByConnection_doesNotCallDeleteLater();
    void testCleanupAfterPersistentConnectionLoop_whenNotManagedByConnection_callsDeleteLater();

    // getPeerAddressAsString() tests
    void testGetPeerAddressAsString_returnsCorrectIPv4Format();
    void testGetPeerAddressAsString_returnsCorrectIPv6Format();

    // sendCurrentPacket() tests
    void testSendCurrentPacket_emitsConnectionStatusWhenDataExists();
    void testSendCurrentPacket_emitsSentPacketWhenDataExists();
    void testSendCurrentPacket_doesNothingWhenNoDataToSend();

    // handleReceiveBeforeSend() tests
    void testHandleReceiveBeforeSend_whenDataReceived_processesAndSendsResponse();
    void testHandleReceiveBeforeSend_whenNoDataReceived_doesNothing();
    void testHandleReceiveBeforeSend_setsCorrectPacketFields();
    void testHandleReceiveBeforeSend_setsSSLWhenSocketIsEncrypted();

    // buildReceivedPacket() tests
    void testBuildReceivedPacket_populatesMetadataAndDrainsData();
    void testBuildReceivedPacket_setsSSLWhenSocketIsEncrypted();
    void testBuildReceivedPacket_handlesNoDataGracefully();

    // responseAfterSend() tests
    void testHandleResponseAfterSend_nonPersistent_emitsDisconnecting();
    void testHandleResponseAfterSend_normalCase_emitsReceivedPacket();
    void testHandleResponseAfterSend_receiveBeforeSendOnlyEmitsWhenDataPresent();
    void testHandleResponseAfterSend_secondReadEmitsResponseAndReplies();

    // shouldBreakPersistentLoop() tests
    void testShouldBreakPersistentLoop_returnsTrueForNonPersistent();
    void testShouldBreakPersistentLoop_returnsFalseForPersistent();

    // resetPacketForPersistentLoop() tests
    void testResetPacketForPersistentLoop();

private:
    void dumpStatusSpy(const QSignalSpy& statusSpy);
};


#endif //PERSISTENTCONNECTIONLOOPTESTS_H
