//
// Created by Tomas Gallucci on 6/9/26.
//

#ifndef INCOMINGTCPTHREADTESTDOUBLE_H
#define INCOMINGTCPTHREADTESTDOUBLE_H
#include "../../../incomingtcpthread.h"
#include "utils/calltracker.h"

class IncomingTcpThreadTestDouble : public IncomingTcpThread, public CallTracker
{
public:
    // Main constructor (used by tests and convenience constructor)
    explicit IncomingTcpThreadTestDouble(PacketSenderQSslSocketInterface* socketInterface,
                               bool isSecure = false,
                               QObject* parent = nullptr)
                                   : IncomingTcpThread(socketInterface, isSecure, parent)
    {

    }

    // Convenience constructor (normal production use)
    explicit IncomingTcpThreadTestDouble(int socketDescriptor,
                               bool isSecure = false,
                               QObject* parent = nullptr)
                                   : IncomingTcpThread(socketDescriptor, isSecure, parent)
    {
        // Don't use this constructor because it will create a RealQSsl object
        // in IncomingTcpThread
    }

    ~IncomingTcpThreadTestDouble() override
    {

    }

    void setSocketForTest(PacketSenderQSslSocketInterface* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    bool shouldStop() const override
    {
        recordCall(CallTracker::SHOULD_STOP_PERSISTENT_CONNECTION_LOOP());

        if (getCallCount(SHOULD_STOP_PERSISTENT_CONNECTION_LOOP()) >= numberOfPersistentConnectionLoopIterationsDesired)
        {
            return true;
        }

        return IncomingTcpThread::shouldStop();
    }

    bool isInterruptionRequested() const override
    {
        recordCall(CallTracker::IS_INTERRUPTION_REQUESTED());
        if (shouldCallIncomingTcpThreadIsInterruptionRequested)
        {
            return IncomingTcpThread::isInterruptionRequested();
        } else
        {
            return isInterruptionRequestedReturnValue;
        }
    }

public:
    bool shouldCallIncomingTcpThreadIsInterruptionRequested = false;
    bool isInterruptionRequestedReturnValue = true;
    int numberOfPersistentConnectionLoopIterationsDesired = 0;

    // ═════════════════════════════════════════════════════════════════════════════
    //                              CALL* SECTION
    // ═════════════════════════════════════════════════════════════════════════════
    //                 All call* methods (incoming command handlers)
    // ═════════════════════════════════════════════════════════════════════════════
    Packet callBuildReceivedPacket()
    {
        return buildReceivedPacket();
    }

    void callSendSmartReplyIfConfigured(Packet& packet)
    {
        return sendSmartReplyIfConfigured(packet);
    }

    void callEmitSSLDiagnosticPackets()
    {
        emitSSLDiagnosticPackets();
    }

    void callPerformSSLHandshakeIfNeeded()
    {
        performSSLHandshakeIfNeeded();
    }

    void callHandleIncomingConnection()
    {
        handleIncomingConnection();
    }

    void callRun()
    {
        run();
    }

    void callPersistentConnectionLoop()
    {
        persistentConnectionLoop();
    }

    bool lastAllowSnakeOilValue;

protected:
    /****************************************************************************************
     *                                                                                      *
     *                                  OVERRIDES SECTION                                   *
     *                                                                                      *
     *  All overridden virtual methods follow this exact pattern:                           *
     *                                                                                      *
     *     1. Increment the corresponding call counter                                      *
     *     2. Record the method name in callHistory (vector<QString>)                       *
     *     3. Forward the call to the real implementation (if set)                          *
     *     4. Return the result                                                             *
     *                                                                                      *
     ****************************************************************************************/

    Packet buildReceivedPacket() override
    {
        recordCall(BUILD_RECEIVED_PACKET());
        return IncomingTcpThread::buildReceivedPacket();
    }

    void sendOutgoingPacket(Packet& packet) override
    {
        recordCall(SEND_OUTGOING_PACKET());
        BaseTcpThread::sendOutgoingPacket(packet);
    }

    void sendSmartReplyIfConfigured(const Packet& packet) override
    {
        recordCall(SEND_SMART_REPLY_IF_CONFIGURED());
        IncomingTcpThread::sendSmartReplyIfConfigured(packet);
    }

    void emitSSLDiagnosticPackets() override
    {
        recordCall(EMIT_SSL_DIAGNOSTIC_PACKETS());
        IncomingTcpThread::emitSSLDiagnosticPackets();
    }

    void performSSLHandshakeIfNeeded() override
    {
        recordCall(PERFORM_SSL_HANDSHAKE_IF_NEEDED());
        IncomingTcpThread::performSSLHandshakeIfNeeded();
    }

    void loadSSLCerts(bool allowSnakeOil) override
    {
        recordCall(LOAD_SSL_CERTS());
        lastAllowSnakeOilValue = allowSnakeOil;

        IncomingTcpThread::loadSSLCerts(allowSnakeOil);
    }

    void loadSnakeOilCerts() override
    {
        recordCall(LOAD_SNAKEOIL_CERTS_());
        IncomingTcpThread::loadSnakeOilCerts();
    }

    void handleIncomingConnection() override
    {
        recordCall(HANDLE_INCOMING_CONNECTION());
        IncomingTcpThread::handleIncomingConnection();
    }

    void run() override
    {
        recordCall(RUN());
        IncomingTcpThread::run();
    }

    void closeConnection() override
    {
        recordCall(CLOSE_CONNECTION());
        IncomingTcpThread::closeConnection();
    }

    void persistentConnectionLoop() override
    {
        recordCall(PERSISTENT_CONNECTION_LOOP());
        IncomingTcpThread::persistentConnectionLoop();
    }

private:
    int shouldStopCallCount = 0;
};

#endif //INCOMINGTCPTHREADTESTDOUBLE_H
