//
// Created by Tomas Gallucci on 6/10/26.
//

#ifndef CALLTRACKER_H
#define CALLTRACKER_H
#include <QString>
#include <QHash>


class CallTracker
{
protected:
    mutable std::vector<QString> callSequence;
    mutable QHash<QString, int> callCounts;

public:
    const std::vector<QString>& getCallSequence() const;

    void recordCall(const QString& methodName) const;
    bool wasMethodCalled(const QString& methodName) const;

    int getCallCount(const QString& methodName) const;

    void clearCallSequence() const;

    // all
    static QString RUN() { return QStringLiteral("run"); }
    static QString BUILD_RECEIVED_PACKET() { return QStringLiteral("buildReceivedPacket"); }


    // BaseTcpThread
    static QString LOAD_SSL_CERTS() { return QStringLiteral("loadSSLCerts"); }
    static QString LOAD_SNAKEOIL_CERTS_() { return QStringLiteral("loadSnakeOilCerts"); }
    static QString CLOSE_CONNECTION() { return QStringLiteral("closeConnection"); }

    // OutgoingTcpThread
    static QString PERSISTENT_CONNECTION_LOOP() { return QStringLiteral("persistentConnectionLoop"); }
    static QString HANDLE_OUTGOING_SSL() { return QStringLiteral("handleOutgoingSSL"); }
    static QString HANDLE_OUTGOING_SSL_HANDSHAKE_FAILURE() { return QStringLiteral("handleOutgoingSSLHandshakeFailure"); }
    static QString HANDLE_OUTGOING_SSL_HANDSHAKE_SUCCESS() { return QStringLiteral("handleOutgoingSSLHandshakeSuccess"); }
    static QString HANDLE_CONNECTION_FAILURE() { return QStringLiteral("handleConnectionFailure"); }
    static QString HANDLE_OUTGOING_PLAIN_TCP() { return QStringLiteral("handleOutgoingPlainTCP"); }
    static QString GET_SMART_RESPONSE_DATA() { return QStringLiteral("getSmartResponseData"); }
    static QString SEND_REPLY_IF_NEEDED() { return QStringLiteral("sendReplyIfNeeded"); }
    static QString SHOULD_SEND_REPLY() { return QStringLiteral("shouldSendReply"); }
    static QString BUILD_REPLY_PACKET() { return QStringLiteral("buildReplyPacket"); }
    static QString WAIT_FOR_AND_PROCESS_INCOMING_DATA() { return QStringLiteral("waitForAndProcessIncomingData"); }
    static QString INTERRUPTABLE_WAIT_FOR_READY_READ() { return QStringLiteral("interruptibleWaitForReadyRead"); }
    static QString HANDLE_PERSISTENT_IDLE_CASE() { return QStringLiteral("handlePersistentIdleCase"); }
    static QString SHOULD_STOP_PERSISTENT_CONNECTION_LOOP() { return QStringLiteral("shouldStopPersistentConnectionLoop"); }
    static QString SHOULD_CONTINUE_PERSISTENT_LOOP() { return QStringLiteral("shouldContinuePersistentLoop"); }
    static QString PROCESS_INCOMING_DATA() { return QStringLiteral("processIncomingData"); }
    static QString BUILD_RECEIVED_PACKET() { return QStringLiteral("buildReceivedPacket"); }
    static QString SEND_OUTGOING_PACKET() { return QStringLiteral("sendOutgoingPacket"); }
    static QString PREPARE_OUTGOING_PACKET() { return QStringLiteral("prepareOutgoingPacket"); }
    static QString OUTGOINGTCPTHREAD_SHUTDOWN() { return QStringLiteral("OutgoingTcpThread shutdown"); }
    static QString OUTGOINGTCPTHREAD_STOP() { return QStringLiteral("OutgoingTcpThread stop"); }


    // IncomingTcpThread
    static QString SEND_SMART_REPLY_IF_CONFIGURED() { return QStringLiteral("sendSmartReplyIfConfigured"); }
    static QString EMIT_SSL_DIAGNOSTIC_PACKETS() { return QStringLiteral("emitSSLDiagnosticPackets"); }
    static QString PERFORM_SSL_HANDSHAKE_IF_NEEDED() { return QStringLiteral("performSSLHandshakeIfNeeded"); }
    static QString HANDLE_INCOMING_CONNECTION() { return QStringLiteral("handleIncomingConnection"); }

    // MockSslSocket
    static QString IGNORE_SSL_ERRORS() { return QStringLiteral("ignoreSslErrors"); }
    static QString START_SERVER_ENCRYPTION() { return QStringLiteral("startServerEncryption"); }
    static QString WAIT_FOR_ENCRYPTED() { return QStringLiteral("waitForEncrypted"); }

};

#endif //CALLTRACKER_H
