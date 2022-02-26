#pragma once

#include <QCoreApplication>
#include "packetnetwork.h"

class MainPacketReceiver : public QObject
{
        Q_OBJECT

    public:
        MainPacketReceiver(QObject *parent);
        bool finished;
        void toTrafficLog(Packet sendpacket);
        void send(Packet packetToSend);
        Packet receivedPacket;


    signals:
        void sendPacket(Packet packetToSend);


    private:
        PacketNetwork * packetNetwork;
        void httpFinished();
};

