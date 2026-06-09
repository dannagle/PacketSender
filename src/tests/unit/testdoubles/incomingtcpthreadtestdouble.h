//
// Created by Tomas Gallucci on 6/9/26.
//

#ifndef INCOMINGTCPTHREADTESTDOUBLE_H
#define INCOMINGTCPTHREADTESTDOUBLE_H
#include "../../../incomingtcpthread.h"

class IncomingTcpThreadTestDouble : public IncomingTcpThread
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

    bool wasMethodCalled(const QString& methodName) const
    {
        return std::find(callSequence.begin(), callSequence.end(), methodName)
               != callSequence.end();
    }

    void setSocketForTest(PacketSenderQSslSocketInterface* newSocket)
    {
        getSocketPtrByReference().reset(newSocket);
    }

    // ═════════════════════════════════════════════════════════════════════════════
    //                              CALL* SECTION
    // ═════════════════════════════════════════════════════════════════════════════
    //                 All call* methods (incoming command handlers)
    // ═════════════════════════════════════════════════════════════════════════════
    Packet callBuildInitialReceivedPacket()
    {
        return buildInitialReceivedPacket();
    }

    void callSendSmartReplyIfConfigured(Packet& packet)
    {
        return sendSmartReplyIfConfigured(packet);
    }

    /****************************************************************************************
     *                                                                                      *
     *                             METHOD CALL COUNTERS                                     *
     *                                                                                      *
     *   All call counters for overridden methods are declared in this section.             *
     *   They are incremented at the beginning of each corresponding override.              *
     *                                                                                      *
     ****************************************************************************************/
    int buildInitialReceivedPacketCallCount = 0;
    int sendOutgoingPacketCallCount = 0;
    int sendSmartReplyIfConfiguredCallCount = 0;

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

    Packet buildInitialReceivedPacket() override
    {
        buildInitialReceivedPacketCallCount++;
        callSequence.push_back("buildInitialReceivedPacket");

        return IncomingTcpThread::buildInitialReceivedPacket();
    }

    void sendOutgoingPacket(Packet& packet) override
    {
        sendOutgoingPacketCallCount++;
        callSequence.push_back("sendOutgoingPacket");

        BaseTcpThread::sendOutgoingPacket(packet);
    }


    void sendSmartReplyIfConfigured(const Packet& packet) override
    {
        sendSmartReplyIfConfiguredCallCount++;
        callSequence.push_back("sendSmartReplyIfConfigured");

        IncomingTcpThread::sendSmartReplyIfConfigured(packet);
    }

private:
    mutable std::vector<QString> callSequence;
};

#endif //INCOMINGTCPTHREADTESTDOUBLE_H
