#include "mainpacketreceiver.h"
#include "globals.h"


MainPacketReceiver::MainPacketReceiver(QObject *parent) :QObject(parent) {
    finished = false;
    packetNetwork = new PacketNetwork(parent);
    packetNetwork->consoleMode = true;

    QDEBUG();


    if (!connect(this->packetNetwork, &PacketNetwork::packetReceived, this, &MainPacketReceiver::toTrafficLog)) {
        QDEBUG() << "packetNetwork packetReceived false";
    }


    if (!connect(this->packetNetwork, &PacketNetwork::packetSent, this, &MainPacketReceiver::toTrafficLog)) {
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


void MainPacketReceiver::readPendingDatagrams()
{
    QDEBUG() << "got a packet";

}

void MainPacketReceiver::initUDP(QString host, int port)
{

    udpSocket = new QUdpSocket(this);
    QHostAddress addy(host);
    udpSocket->bind(addy, port);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &MainPacketReceiver::readPendingDatagrams);
}



void MainPacketReceiver::initSSL(QString host, int port)
{
    sslSocket = new QSslSocket(this);
    QHostAddress addy(host);
    sslSocket->bind(addy, port);

    connect(sslSocket, &QSslSocket::readyRead,
            this, &MainPacketReceiver::readPendingDatagrams);

}



void MainPacketReceiver::send(Packet packetToSend) {
    QDEBUG();

    emit sendPacket(packetToSend);
    QDEBUG();
}


void MainPacketReceiver::toTrafficLog(Packet sendpacket) {

    if(sendpacket.toIP.toLower() == "you") {
        receivedPacket = sendpacket;
    }
}
