//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCHPTHREADTESTDOUBLE_H
#define OUTGOINGTCHPTHREADTESTDOUBLE_H

#include "realqsslsocket.h"
#include "../../tcpThreads/outgoingtcpthread.h"
#include "utils/calltracker.h"

class OutgoingTcpThreadTestDouble : public OutgoingTcpThread, public CallTracker
{
    Q_OBJECT

signals:
    void outgoingThreadTestDoubleAboutToShutdown();   // emitted when shutdown/stop is requested
    void threadTestDoubleDestructorCalled();         // emitted when the object is actually deleted
    void runStarted(Qt::HANDLE threadId);           // emitted when run is called so we can get the thread id

public:
    explicit OutgoingTcpThreadTestDouble(PacketSenderQSslSocketInterface* socketInterface,
                                         const Packet& packet)
        : OutgoingTcpThread(socketInterface, packet) {}

    // Convenience constructor
    explicit OutgoingTcpThreadTestDouble(const Packet& packetToSend)
        : OutgoingTcpThreadTestDouble(new MockSslSocket(), packetToSend)   // delegates to main constructor
    {}

    explicit OutgoingTcpThreadTestDouble(const Packet& packetToSend, QObject *parent)
        : OutgoingTcpThread(new MockSslSocket(), packetToSend, parent)
    {}

    ~OutgoingTcpThreadTestDouble() override
    {
        emit threadTestDoubleDestructorCalled();
    }

    Qt::HANDLE getThreadIdCapturedInRun() const
    {
        return threadId;
    }

    Packet& getSendPacketByReference()
    {
        return this->sendPacket;
    }

    Packet& getCommandLineReplyPacketByReference()
    {
        return this->commandLineReplyPacket;
    }

    void setSocketForTest(PacketSenderQSslSocketInterface* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    void stop() override
    {
        simulateRequestInterruptionCalled = true;

        recordCall(CallTracker::OUTGOINGTCPTHREAD_STOP());
        OutgoingTcpThread::stop();
    }

    [[nodiscard]] bool shouldStop() const override
    {
        QDEBUG() << "simulateRequestInterruptionCalled in test double: " << simulateRequestInterruptionCalled;
        return simulateRequestInterruptionCalled;
    }

    void shutdown() override
    {
        emit outgoingThreadTestDoubleAboutToShutdown();
        recordCall(CallTracker::OUTGOINGTCPTHREAD_SHUTDOWN());
        OutgoingTcpThread::shutdown();
    }

    bool loopExitedCleanly() const
    {
        return !shouldContinuePersistentLoop() &&
               shouldContinuePersistentConnectionLoopCallCount > 0;   // we actually entered the loop at least once
    }

    int forceExitAfterNIterations = 2;
    mutable int persistentConnectionLoopIterationsCount = 0;
    mutable int shouldContinuePersistentConnectionLoopCallCount = 0;
    int baseSendOutgoingPacketCallCount = 0;
    int sleepCallCount = 0;

    void callPrepareOutgoingSendPacket()
    {
        prepareOutgoingPacket();
    }

    bool callIsValidForSending(Packet& packet, QString* errorMessage)
    {
        return isValidForSending(packet, errorMessage);
    }

    void callCloseConnection()
    {
        closeConnection();
    }

    void callRun()
    {
        OutgoingTcpThread::run();
    }

    bool callShouldContinuePersistentConnectionLoop()
    {
        return shouldContinuePersistentLoop();
    }

    bool callShouldStopPersistentConnectionLoop()
    {
        return shouldStopPersistentConnectionLoop();
    }

    void callHandlePersistentIdleCase(int idleThresholdms = 200)
    {
        handlePersistentIdleCase(idleThresholdms);
    }

    void callPersistentConnectionLoop()
    {
        persistentConnectionLoop();
    }

    Packet callBuildReceivedPacket()
    {
        return buildReceivedPacket();
    }

    void callProcessIncomingData()
    {
        processIncomingData();
    }

    void callWaitForAndProcessIncomingData()
    {
        waitForAndProcessIncomingData();
    }

    Packet callBuildReplyPacket(const Packet& receivedPacket, const QByteArray& responseData)
    {
        return buildReplyPacket(receivedPacket, responseData);
    }

    bool callShouldSendReply() const
    {
        return shouldSendReply();
    }

    void callSendReplyIfNeeded(const Packet& p)
    {
        sendReplyIfNeeded(p);
    }

    QByteArray callGetSmartResponseData(const Packet& receivedPacket)
    {
        return getSmartResponseData(receivedPacket);
    }

    bool callHandleOutgoingPlainTCP()
    {
        return handleOutgoingPlainTCP();
    }

    void callHandleConnectionFailure()
    {
        handleConnectionFailure();
    }

    void callLoadSSLCerts(bool allowSnakeOil)
    {
        loadSSLCerts(allowSnakeOil);
    }

    void callLoadSnakeOilCerts()
    {
        loadSnakeOilCerts();
    }

    void callHandleOutgoingSSLHandshakeSuccess()
    {
        handleOutgoingSSLHandshakeSuccess();
    }

    void callHandleOutgoingSSLHandshakeFailure()
    {
        handleOutgoingSSLHandshakeFailure();
    }

    bool callHandleOutgoingSSL()
    {
        return handleOutgoingSSL();
    }

    void setConsoleMode(bool isConsoleMode) { consoleMode = isConsoleMode; }
    std::optional<bool> lastAllowSnakeOilValue;

protected:

    void prepareOutgoingPacket() override
    {
        recordCall(PREPARE_OUTGOING_PACKET());
        OutgoingTcpThread::prepareOutgoingPacket();
    }

    bool isValidForSending(Packet& packet, QString* errorMessage) const override
    {
        recordCall(OUTGOINGTCPTHREAD_ISVALIDFORSENDING());
        return OutgoingTcpThread::isValidForSending(packet, errorMessage);
    }

    void sendOutgoingPacket() override
    {
        recordCall(SEND_OUTGOING_PACKET());
        OutgoingTcpThread::sendOutgoingPacket();
    }

    void sendOutgoingPacket(Packet &packet) override
    {
        baseSendOutgoingPacketCallCount++;
        callSequence.push_back("BaseTcpThread::sendOutgoingPacket");
        BaseTcpThread::sendOutgoingPacket(packet);
    }

    void closeConnection() override
    {
        recordCall(CLOSE_CONNECTION());
        OutgoingTcpThread::closeConnection();
    }

    void sleep(unsigned long usecs) override
    {
        sleepCallCount++;
        recordCall("usleep " + QString::number(usecs) + " usecs");
        OutgoingTcpThread::sleep(usecs);
    }

    Packet buildReceivedPacket() override
    {
        recordCall(BUILD_RECEIVED_PACKET());
        return OutgoingTcpThread::buildReceivedPacket();
    }

    void processIncomingData() override
    {
        recordCall(PROCESS_INCOMING_DATA());
        OutgoingTcpThread::processIncomingData();
    }

    bool shouldContinuePersistentLoop() const override
    {
        recordCall(SHOULD_CONTINUE_PERSISTENT_LOOP());
        shouldContinuePersistentConnectionLoopCallCount++;
        persistentConnectionLoopIterationsCount++;

        if (persistentConnectionLoopIterationsCount >= forceExitAfterNIterations) {
            qDebug() << "Test double: forcing exit after iteration" << shouldContinuePersistentConnectionLoopCallCount;
            return false;
        }

        return OutgoingTcpThread::shouldContinuePersistentLoop();
    }

    bool shouldStopPersistentConnectionLoop() const override
    {
        recordCall(SHOULD_STOP_PERSISTENT_CONNECTION_LOOP());
        return OutgoingTcpThread::shouldStopPersistentConnectionLoop();
    }

    void handlePersistentIdleCase(int idleThresholdMs) override
    {
        recordCall(HANDLE_PERSISTENT_IDLE_CASE());
        OutgoingTcpThread::handlePersistentIdleCase(idleThresholdMs);
    }

    bool interruptibleWaitForReadyRead(int timeoutMs) override
    {
        recordCall(INTERRUPTABLE_WAIT_FOR_READY_READ());

        return BaseTcpThread::interruptibleWaitForReadyRead(timeoutMs);
    }

    void waitForAndProcessIncomingData() override
    {
        recordCall(WAIT_FOR_AND_PROCESS_INCOMING_DATA());

        OutgoingTcpThread::waitForAndProcessIncomingData();
    }


    Packet buildReplyPacket(const Packet& receivedPacket, const QByteArray& responseData) override
    {
        recordCall(BUILD_REPLY_PACKET());

        return OutgoingTcpThread::buildReplyPacket(receivedPacket, responseData);
    }

    bool shouldSendReply() const override
    {
        recordCall(SHOULD_SEND_REPLY());

        return OutgoingTcpThread::shouldSendReply();
    }

    void sendReplyIfNeeded(const Packet& receivedPacket) override
    {
        recordCall(SEND_REPLY_IF_NEEDED());

        OutgoingTcpThread::sendReplyIfNeeded(receivedPacket);
    }

    QByteArray getSmartResponseData(const Packet& receivedPacket) override
    {
        recordCall(GET_SMART_RESPONSE_DATA());

        return OutgoingTcpThread::getSmartResponseData(receivedPacket);
    }

    bool handleOutgoingPlainTCP() override
    {
        recordCall(HANDLE_OUTGOING_PLAIN_TCP());

        return OutgoingTcpThread::handleOutgoingPlainTCP();
    }

    void handleConnectionFailure() override
    {
        recordCall(HANDLE_CONNECTION_FAILURE());

        OutgoingTcpThread::handleConnectionFailure();
    }

    void loadSSLCerts(bool allowSnakeOil) override
    {
        recordCall(LOAD_SSL_CERTS());
        lastAllowSnakeOilValue = allowSnakeOil;

        OutgoingTcpThread::loadSSLCerts(allowSnakeOil);
    }

    void loadSnakeOilCerts() override
    {
        recordCall(LOAD_SNAKEOIL_CERTS_());

        OutgoingTcpThread::loadSnakeOilCerts();
    }

    void handleOutgoingSSLHandshakeSuccess() override
    {
        recordCall(HANDLE_OUTGOING_SSL_HANDSHAKE_SUCCESS());

        OutgoingTcpThread::handleOutgoingSSLHandshakeSuccess();
    }

    void handleOutgoingSSLHandshakeFailure() override
    {
        recordCall(HANDLE_OUTGOING_SSL_HANDSHAKE_FAILURE());

        OutgoingTcpThread::handleOutgoingSSLHandshakeFailure();
    }

    bool handleOutgoingSSL() override
    {
        recordCall(HANDLE_OUTGOING_SSL());

        return OutgoingTcpThread::handleOutgoingSSL();
    }

    void persistentConnectionLoop() override
    {
        recordCall(PERSISTENT_CONNECTION_LOOP());

        OutgoingTcpThread::persistentConnectionLoop();
    }

    void run() override
    {
        emit runStarted(currentThreadId());
        threadId = currentThreadId();
        recordCall(RUN());
        OutgoingTcpThread::run();
    }

private:
    bool simulateRequestInterruptionCalled = false;
    Qt::HANDLE threadId = nullptr;
};

#endif //OUTGOINGTCHPTHREADTESTDOUBLE_H
