//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCHPTHREADTESTDOUBLE_H
#define OUTGOINGTCHPTHREADTESTDOUBLE_H
#include <qtestcase.h>

#include "realqsslsocket.h"
#include "../../outgoingtcpthread.h"

class OutgoingTcpThreadTestDouble : public OutgoingTcpThread
{
public:
    explicit OutgoingTcpThreadTestDouble(PacketSenderQSslSocketInterface* socketInterface,
                                         const Packet& packet)
        : OutgoingTcpThread(socketInterface, packet) {}

    // Convenience constructor
    explicit OutgoingTcpThreadTestDouble(const Packet& packetToSend)
        : OutgoingTcpThreadTestDouble(new MockSslSocket(), packetToSend)   // delegates to main constructor
    {}

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
    }

    [[nodiscard]] bool shouldStop() const override
    {
        QDEBUG() << "simulateRequestInterruptionCalled in test double: " << simulateRequestInterruptionCalled;
        return simulateRequestInterruptionCalled;
    }

    bool loopExitedCleanly() const
    {
        return !shouldContinuePersistentLoop() &&
               shouldContinuePersistentConnectionLoopCallCount > 0;   // we actually entered the loop at least once
    }

    int forceExitAfterNIterations = 2;

    int prepareOutgoingPacketCallCount = 0;
    int sendOutgoingPacketCallCount = 0;
    int baseSendOutgoingPacketCallCount = 0;
    int closeConnectionCallCount = 0;
    int sleepCallCount = 0;
    mutable int shouldContinuePersistentConnectionLoopCallCount = 0;
    mutable int shouldStopPersistentConnectionLoopCallCount = 0;
    mutable int persistentConnectionLoopIterationsCount = 0;
    int handlePersistentIdleCaseCallCount = 0;
    int buildReceivedPacketCallCount = 0;
    int processIncomingDataCallCount = 0;
    int waitForAndProcessIncomingDataCallCount = 0;
    int interruptibleWaitForReadyReadCallCount = 0;
    int buildReplyPacketCallCount = 0;
    mutable int shouldSendReplyCallCount = 0;
    mutable int sendReplyIfNeededCallCount = 0;
    int getSmartResponseDataCallCount = 0;
    int handleOutgoingPlainTcpCallCount = 0;
    int handleConnectionFailureCallCount = 0;

    bool wasMethodCalled(const QString& methodName) const
    {
        return std::find(callSequence.begin(), callSequence.end(), methodName)
               != callSequence.end();
    }

    const std::vector<QString>& getCallSequence() const { return callSequence; }

    void callPrepareOutgoingSendPacket()
    {
        prepareOutgoingPacket();
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

    void setConsoleMode(bool isConsoleMode) { consoleMode = isConsoleMode; }

protected:

    void prepareOutgoingPacket() override
    {
        prepareOutgoingPacketCallCount++;
        callSequence.push_back("prepareOutgoingPacket");
        OutgoingTcpThread::prepareOutgoingPacket();
    }

    void sendOutgoingPacket() override
    {
        sendOutgoingPacketCallCount++;
        callSequence.push_back("sendOutgoingPacket");
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
        closeConnectionCallCount++;
        callSequence.push_back("closeConnection");
        OutgoingTcpThread::closeConnection();
    }

    void sleep(unsigned long usecs) override
    {
        sleepCallCount++;
        callSequence.push_back("usleep " + QString::number(usecs) + " usecs");
        OutgoingTcpThread::sleep(usecs);
    }

    Packet buildReceivedPacket() override
    {
        buildReceivedPacketCallCount++;
        callSequence.push_back("buildReceivedPacket");
        return OutgoingTcpThread::buildReceivedPacket();
    }

    void processIncomingData() override
    {
        processIncomingDataCallCount++;
        callSequence.push_back("processIncomingData");
        OutgoingTcpThread::processIncomingData();
    }

    bool shouldContinuePersistentLoop() const override
    {
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
        shouldStopPersistentConnectionLoopCallCount++;
        return OutgoingTcpThread::shouldStopPersistentConnectionLoop();
    }

    void handlePersistentIdleCase(int idleThresholdMs) override
    {
        handlePersistentIdleCaseCallCount++;
        OutgoingTcpThread::handlePersistentIdleCase(idleThresholdMs);
    }

    bool interruptibleWaitForReadyRead(int timeoutMs) override
    {
        interruptibleWaitForReadyReadCallCount++;
        callSequence.push_back("interruptibleWaitForReadyRead");

        return BaseTcpThread::interruptibleWaitForReadyRead(timeoutMs);
    }

    void waitForAndProcessIncomingData() override
    {
        waitForAndProcessIncomingDataCallCount++;
        callSequence.push_back("waitForAndProcessIncomingData");

        OutgoingTcpThread::waitForAndProcessIncomingData();
    }


    Packet buildReplyPacket(const Packet& receivedPacket, const QByteArray& responseData) override
    {
        buildReplyPacketCallCount++;
        callSequence.push_back("buildReplyPacket");

        return OutgoingTcpThread::buildReplyPacket(receivedPacket, responseData);
    }

    bool shouldSendReply() const override
    {
        shouldSendReplyCallCount++;
        callSequence.push_back("shouldSendReply");

        return OutgoingTcpThread::shouldSendReply();
    }

    void sendReplyIfNeeded(const Packet& receivedPacket) override
    {
        sendReplyIfNeededCallCount++;
        callSequence.push_back("sendReplyIfNeeded");

        OutgoingTcpThread::sendReplyIfNeeded(receivedPacket);
    }

    QByteArray getSmartResponseData(const Packet& receivedPacket) override
    {
        getSmartResponseDataCallCount++;
        callSequence.push_back("getSmartResponseData");

        return OutgoingTcpThread::getSmartResponseData(receivedPacket);
    }

    bool handleOutgoingPlainTCP() override
    {
        handleOutgoingPlainTcpCallCount++;
        callSequence.push_back("handleOutgoingPlainTCP");

        return OutgoingTcpThread::handleOutgoingPlainTCP();
    }

    void handleConnectionFailure() override
    {
        handleConnectionFailureCallCount++;
        callSequence.push_back("handleConnectionFailure");

        OutgoingTcpThread::handleConnectionFailure();
    }

private:
    mutable std::vector<QString> callSequence;
    bool simulateRequestInterruptionCalled = false;
};

#endif //OUTGOINGTCHPTHREADTESTDOUBLE_H
