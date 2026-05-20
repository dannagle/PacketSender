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
    explicit OutgoingTcpThread(QSslSocket* socket,
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
    void closeConnection() override;

    // persistentConnectionLoop() methods
    virtual bool shouldContinuePersistentLoop() const;
    virtual bool shouldStopPersistentConnectionLoop() const;
    virtual void handlePersistentIdleCase(int idleThresholdMs);
    void persistentConnectionLoop();

    Packet      sendPacket;
    Packet      replyPacket;

    bool        consoleMode = false;
    bool        persistent = false;
    bool        receiveBeforeSend = false;
    int         delayAfterConnect = 0;

private:
    void outgoingConnectionDebugMessage(bool connectSuccess);
    void idleDebugMessage(bool isIdleCondition);
    mutable std::optional<QDateTime> lastIdleStatusEmitTime;
};


#endif //OUTGOINGTCPTHREAD_H
