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
    while (udpSocket->hasPendingDatagrams()) {

        QHostAddress sender;
        int senderPort;

        QNetworkDatagram theDatagram = udpSocket->receiveDatagram(10000000);
        QByteArray datagram = theDatagram.data();
        sender =  theDatagram.senderAddress();
        senderPort = theDatagram.senderPort();

        QDEBUG() << "data size is" << datagram.size() << sender << senderPort;

    }

}

bool MainPacketReceiver::initUDP(QString host, int port)
{

    udpSocket = new QUdpSocket(this);
    QHostAddress addy(host);
    udpSocket->bind(addy, port);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &MainPacketReceiver::readPendingDatagrams);

    return udpSocket->state() == QAbstractSocket::BoundState;
}



bool MainPacketReceiver::initSSL(QString host, int port)
{
    tcpServer = new ThreadedTCPServer(nullptr);
    tcpServer->init(port, false, host);

    return tcpServer->isListening();
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
