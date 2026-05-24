//
// Created by Tomas Gallucci on 5/9/26.
//

#ifndef OUTGOINGTCHPTHREADTESTDOUBLE_H
#define OUTGOINGTCHPTHREADTESTDOUBLE_H
#include <qtestcase.h>

#include "../../outgoingtcpthread.h"

class OutgoingTcpThreadTestDouble : public OutgoingTcpThread
{
public:
    explicit OutgoingTcpThreadTestDouble(
        QSslSocket* socket, const Packet& packetToSend): OutgoingTcpThread(socket, packetToSend)
    {}

    // Convenience constructor
    explicit OutgoingTcpThreadTestDouble(const Packet& packetToSend)
        : OutgoingTcpThreadTestDouble(new QSslSocket(), packetToSend)   // delegates to main constructor
    {}

    Packet& getSendPacketByReference()
    {
        return this->sendPacket;
    }

    void setSocketForTest(QSslSocket* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    bool isSocketValid() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket()))
        {
            return mock->isValid();
        }

        return BaseTcpThread::isSocketValid();
    }

    QAbstractSocket::SocketState getSocketState() const override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket()))
        {
            return mock->getMockState();
        }

        return BaseTcpThread::getSocketState();
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

    const std::vector<QString>& getCallSequence() const { return callSequence; }
    void resetCallTracking()
    {
        prepareOutgoingPacketCallCount
            = sendOutgoingPacketCallCount
            = closeConnectionCallCount
            = sleepCallCount
            = 0;
        callSequence.clear();
    }

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

    QByteArray readSocketData() override
    {
        if (const MockSslSocket *mock = qobject_cast<const MockSslSocket*>(getSocket())) {
            return mock->getMockReadData();
        }

        return BaseTcpThread::readSocketData();
    }

    Packet buildReceivedPacket() override
    {
        buildReceivedPacketCallCount++;
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

    bool interruptibleWaitForReadyRead(int timeoutMs)
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


private:
    std::vector<QString> callSequence;
    bool simulateRequestInterruptionCalled = false;
};

#endif //OUTGOINGTCHPTHREADTESTDOUBLE_H
