#include "mainpacketreceiver.h"



MainPacketReceiver::MainPacketReceiver(QObject *parent) :QObject(parent) {
    finished = false;
    packetNetwork = new PacketNetwork(parent);
    packetNetwork->consoleMode = true;

    QDEBUG();

    if (!connect(this->packetNetwork, &PacketNetwork::packetSent, this, &MainPacketReceiver::toTrafficLog)) {
        QDEBUG() << "packetNetwork packetSent false";
    }
    QDEBUG();

    if (!connect(this->packetNetwork, &PacketNetwork::packetReceived, this, &MainPacketReceiver::toTrafficLog)) {
        QDEBUG() << "packetNetwork packetReceived false";
    }
    QDEBUG();


    if (!connect(this, &MainPacketReceiver::sendPacket, packetNetwork, &PacketNetwork::packetToSend)) {
        QDEBUG() << "MainPacketReceiver sendPacket false";
    }



    if (!connect(packetNetwork->http, &QNetworkAccessManager::finished, this, &MainPacketReceiver::httpFinished)) {
        QDEBUG() << "httpFinished http false";
    }

}


void MainPacketReceiver::httpFinished()
{
    QDEBUG() << "finished http.";
    finished = true;

}

void MainPacketReceiver::send(Packet packetToSend) {
    QDEBUG();

    emit sendPacket(packetToSend);
    QDEBUG();
}


void MainPacketReceiver::toTrafficLog(Packet sendpacket) {
    QDEBUGVAR(sendpacket.asciiString());
}
