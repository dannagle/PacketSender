//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCPTHREAD_H
#define OUTGOINGTCPTHREAD_H

#include <QObject>
#include "basetcpthread.h"

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
    // void run() override;

    Packet      sendPacket;
    Packet      replyPacket;

    bool        consoleMode = false;
    bool        persistent = false;
    bool        receiveBeforeSend = false;
    int         delayAfterConnect = 0;

};


#endif //OUTGOINGTCPTHREAD_H
