//
// Created by Tomas Gallucci on 6/9/26.
//

#ifndef INCOMINGTCPTHREADTESTDOUBLE_H
#define INCOMINGTCPTHREADTESTDOUBLE_H
#include "../../../incomingtcpthread.h"
#include "utils/calltracker.h"
#include "testdoubles/MockSslSocket.h"

class IncomingTcpThreadTestDouble : public IncomingTcpThread, public CallTracker
{
    Q_OBJECT
public:
    // Main constructor (used by tests and convenience constructor)
    explicit IncomingTcpThreadTestDouble(PacketSenderQSslSocketInterface* socketInterface,
                               bool isSecure = false,
                               bool isPersistent = false,
                               QObject* parent = nullptr)
                                   : IncomingTcpThread(socketInterface, isSecure, isPersistent, parent)
    {

    }

    // Convenience constructor (normal production use)
    explicit IncomingTcpThreadTestDouble(int socketDescriptor,
                               bool isSecure = false,
                               bool isPersistent = false,
                               QObject* parent = nullptr)
                                   : IncomingTcpThread(socketDescriptor, isSecure, isPersistent, parent)
    {
        // Don't use this constructor because it will create a RealQSsl object
        // in IncomingTcpThread
    }

    explicit IncomingTcpThreadTestDouble(int socketDescriptor, QObject* parent = nullptr)
    : IncomingTcpThreadTestDouble(createSocketWithDescriptorInTestDouble(socketDescriptor), false, false, parent)
    {
        QDEBUG() << "used correct constructor";
    }

    static PacketSenderQSslSocketInterface* createSocketWithDescriptorInTestDouble(int socketDescriptor)
    {

        qDebug() << "socketDescriptor in helper method: " << socketDescriptor;

        auto mockSslSocket = std::make_unique<MockSslSocket>();

        // IMPORTANT: Set the descriptor BEFORE passing to base
        if (!mockSslSocket->setSocketDescriptor(socketDescriptor)) {
            // handle error - e.g. throw or log
            throw std::runtime_error("Failed to set socket descriptor on QSslSocket");
        }

        return mockSslSocket.release();
    }

    ~IncomingTcpThreadTestDouble() override
    {
        destructorCalled();
    }

    void setSocketForTest(PacketSenderQSslSocketInterface* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    void setPersistent(bool isPersistent)
    {
        persistent = isPersistent;
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

    void shutdown() override
    {
        qDebug() << "Shutting down IncomingTcpThreadTestDouble...";
        emit shutdownCalled();
        recordCall(INCOMINGTCPTHREAD_SHUTDOWN());
        IncomingTcpThread::shutdown();
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

    bool shouldCallIncomingTcpThreadIsInterruptionRequested = false;
    bool isInterruptionRequestedReturnValue = true;
    int numberOfPersistentConnectionLoopIterationsDesired = 0;

    Qt::HANDLE getThreadIdCapturedInRun() const
    {
        return threadId;
    }

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

signals:
    void shutdownCalled();
    void destructorCalled();
    // void threadTestDoubleDestructorCalled();         // emitted when the object is actually deleted
    void runStarted(Qt::HANDLE threadId);           // emitted when run is called so we can get the thread id

protected:
    /****************************************************************************************
     *                                                                                      *
     *                                  OVERRIDES SECTION                                   *
     *                                                                                      *
     *  All overridden virtual methods follow this exact pattern:                           *
     *                                                                                      *
     *     1. Record the method call                                                        *
     *     2. Forward the call to the real implementation (if set)                          *
     *     3. Return the result                                                             *
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
        emit runStarted(currentThreadId());
        threadId = currentThreadId();
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
    Qt::HANDLE threadId = nullptr;
};

#endif //INCOMINGTCPTHREADTESTDOUBLE_H
