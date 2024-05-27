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

        QString output = MainPacketReceiver::datagramOutput(udpSocket->receiveDatagram(10000000), false);

        QTextStream out(stdout);
        out << output << Qt::endl;
        out.flush();

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

QString MainPacketReceiver::datagramOutput(QNetworkDatagram theDatagram, bool quiet)
{
    QString output;
    QTextStream out(&output);

    QHostAddress sender;
    int senderPort;

    QByteArray recvData = theDatagram.data();
    sender =  theDatagram.senderAddress();
    senderPort = theDatagram.senderPort();

    QString hexString = Packet::byteArrayToHex(recvData);
    if (quiet) {
        out << "\n" << hexString;
    } else {
        out << "\nFrom: " << sender.toString() << ", Port:" << senderPort;
        out << "\nResponse Time:" << QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        out << "\nResponse HEX:" << hexString;
        out << "\nResponse ASCII:" << Packet::hexToASCII(hexString);
    }

    return output;
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
