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
        void readPendingDatagrams();

        //This is only used in CLI Mode
        QUdpSocket * udpSocket;
        QSslSocket * sslSocket;
        void initUDP(QString host, int port);
        void initSSL(QString host, int port);

    signals:
        void sendPacket(Packet packetToSend);


    private:
        PacketNetwork * packetNetwork;
        void httpFinished();
};

