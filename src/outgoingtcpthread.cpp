//
// Created by Tomas Gallucci on 5/9/26.
//

#include "outgoingtcpthread.h"

OutgoingTcpThread::OutgoingTcpThread(QSslSocket* socket, const Packet& packetToSend, QObject* parent)
:BaseTcpThread(socket, parent), sendPacket(packetToSend)
{
    if (packetToSend.toIP.isEmpty()) {
        throw std::invalid_argument("OutgoingTcpThread: packetToSend.toIP cannot be empty");
    }

    const bool destinationPortHasBeenSet = packetToSend.port > 0;
    if (!destinationPortHasBeenSet) {
        throw std::invalid_argument("OutgoingTcpThread: packetToSend.port must be set to a positive integer value");
    }
}

OutgoingTcpThread::~OutgoingTcpThread()
{
}

QString OutgoingTcpThread::getDestinationAddress() const
{
    return sendPacket.toIP;
}

unsigned int OutgoingTcpThread::getDestinationPort() const
{
    return sendPacket.port;
}

bool OutgoingTcpThread::isValid() const
{
    if (sendPacket.toIP.isEmpty() || sendPacket.port == 0)
    {
        return false;
    }
    return BaseTcpThread::isValid();
}
