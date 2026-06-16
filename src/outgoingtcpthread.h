//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCPTHREAD_H
#define OUTGOINGTCPTHREAD_H

#include <QObject>
#include "basetcpthread.h"
#include "packet.h"

/**
 * Handles outgoing (client) TCP connections - both one-shot and persistent.
 */
class OutgoingTcpThread : public BaseTcpThread
{
    Q_OBJECT

public:
    /**
     * Main constructor - takes ownership of the socket.
     * This is the primary constructor used by tests and low-level code.
     */
    OutgoingTcpThread(PacketSenderQSslSocketInterface* socketInterface,
                                const Packet& packetToSend,
                                QObject* parent = nullptr);

    /**
     * Convenience constructor - creates the socket internally.
     * This is what most production code (Connection) will use.
     */
    explicit OutgoingTcpThread(const Packet& packetToSend, QObject* parent = nullptr);

    ~OutgoingTcpThread() override;

    // Getters
    [[nodiscard]] QString getDestinationAddress() const;
    [[nodiscard]] unsigned int getDestinationPort() const;
    [[nodiscard]] bool isValid() const override;

protected:
    void run() override;
    virtual void prepareOutgoingPacket();
    virtual void sendOutgoingPacket();

    virtual Packet buildReplyPacket(const Packet &receivedPacket, const QByteArray &responseData);

    // SSL reaction methods
    virtual void handleOutgoingSSLHandshakeSuccess();
    virtual void handleOutgoingSSLHandshakeFailure();
    virtual bool handleOutgoingSSL();

    // non-ssl path
    virtual bool handleOutgoingPlainTCP();

    // applicable to both
    virtual void handleConnectionFailure();

    // persistentConnectionLoop() methods
    virtual bool shouldContinuePersistentLoop() const;
    virtual bool shouldStopPersistentConnectionLoop() const;
    virtual void handlePersistentIdleCase(int idleThresholdMs);
    virtual Packet buildReceivedPacket();
    virtual QByteArray getSmartResponseData(const Packet& receivedPacket);
    virtual void processIncomingData();
    virtual void waitForAndProcessIncomingData();
    virtual bool shouldSendReply() const;
    virtual void sendReplyIfNeeded(const Packet& receivedPacket);
    virtual void persistentConnectionLoop();

    Packet      sendPacket;
    Packet      commandLineReplyPacket;

    bool        consoleMode = false;
    bool        receiveBeforeSend = false;
    int         delayAfterConnect = 0;

private:
    void outgoingConnectionDebugMessage(bool connectSuccess);
    void idleDebugMessage(bool isIdleCondition) const;
    mutable std::optional<QDateTime> lastIdleStatusEmitTime;
};


#endif //OUTGOINGTCPTHREAD_H
