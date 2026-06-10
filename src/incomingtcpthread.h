//
// Created by Tomas Gallucci on 6/8/26.
//

#ifndef INCOMINGTCPTHREAD_H
#define INCOMINGTCPTHREAD_H
#include <QObject>

#include "basetcpthread.h"
#include "packetsenderqsslsocketinterface.h"


class IncomingTcpThread : public BaseTcpThread
{
    Q_OBJECT

public:

    // Main constructor (used by tests and convenience constructor)
    explicit IncomingTcpThread(PacketSenderQSslSocketInterface* socketInterface,
                               bool isSecure = false,
                               QObject* parent = nullptr);

    // Convenience constructor (normal production use)
    explicit IncomingTcpThread(int socketDescriptor,
                               bool isSecure = false,
                               QObject* parent = nullptr);

    ~IncomingTcpThread() override;

    [[nodiscard]] int getSocketDescriptor() const { return socketInterface->getSocketDescriptor(); }


protected:
    void closeConnection() override;
    virtual Packet buildInitialReceivedPacket();
    virtual void sendSmartReplyIfConfigured(const Packet& receivedPacket);
    virtual void emitSSLDiagnosticPackets();

    //     void run() override;
    //
    //     virtual void handleIncomingConnection();
    //     virtual void performSSLHandshakeIfNeeded();
    //     virtual Packet buildInitialReceivedPacket();

    static PacketSenderQSslSocketInterface* createSocketWithDescriptor(int socketDescriptor);
};


#endif //INCOMINGTCPTHREAD_H
