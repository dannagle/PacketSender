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
        ThreadedTCPServer * tcpServer;
        bool initUDP(QString host, int port);
        bool initSSL(QString host, int port, bool encrypted);

        static QString datagramOutput(QNetworkDatagram theDatagram, bool quiet = false);
    signals:
        void sendPacket(Packet packetToSend);
        //void receivedPacket(Packet packetReceived);


    private:
        PacketNetwork * packetNetwork;
        void httpFinished();
};

