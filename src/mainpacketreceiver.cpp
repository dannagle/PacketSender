#include "mainpacketreceiver.h"
#include "globals.h"


MainPacketReceiver::MainPacketReceiver(QObject *parent) :QObject(parent) {
    finished = false;
    packetNetwork = new PacketNetwork(parent);
    packetNetwork->consoleMode = true;
    packetReply.clear();



    if (!connect(this->packetNetwork, &PacketNetwork::packetReceived, this, &MainPacketReceiver::toTrafficLog)) {
        QDEBUG() << "packetNetwork packetReceived false";
    }


    if (!connect(this->packetNetwork, &PacketNetwork::packetSent, this, &MainPacketReceiver::toTrafficLog)) {
        QDEBUG() << "packetNetwork packetReceived false";
    }


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

        QNetworkDatagram datagram = udpSocket->receiveDatagram(10000000);


        QTextStream out(stdout);
        QString output = MainPacketReceiver::datagramOutput(datagram, false);
        out << output << Qt::endl;
        out.flush();
        output.clear();

        if(!packetReply.hexString.isEmpty()) {

            Packet sendit = packetReply;
            sendit.tcpOrUdp = "UDP";
            sendit.fromIP = "You (Response)";
            sendit.toIP = datagram.senderAddress().toString();
            sendit.port = datagram.senderPort();
            QString data = Packet::macroSwap(packetReply.asciiString());
            sendit.hexString = Packet::ASCIITohex(data);

            QHostAddress resolved = PacketNetwork::resolveDNS(sendit.toIP);

            udpSocket->writeDatagram(sendit.getByteArray(), resolved, sendit.port);


            out << "\nFrom: " << sendit.fromIP << ", Port:" << sendit.port;
            out << "\nResponse Time:" << QDateTime::currentDateTime().toString(DATETIMEFORMAT);
            out << "\nResponse HEX:" << sendit.hexString;
            out << "\nResponse ASCII:" << sendit.asciiString();
            out << Qt::endl;

        }

        out.flush();

    }

}

bool MainPacketReceiver::initUDP(QString host, int port)
{
    udpSocket = new QUdpSocket(this);
    QHostAddress addy(host);
    if(host == "any" || addy.isNull()) {
        udpSocket->bind(QHostAddress::Any, port);
    } else {
        udpSocket->bind(addy, port);
    }

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &MainPacketReceiver::readPendingDatagrams);

    return udpSocket->state() == QAbstractSocket::BoundState;
}



bool MainPacketReceiver::initSSL(QString host, int port, bool encrypted)
{
    tcpServer = new ThreadedTCPServer(nullptr);
    tcpServer->init(port, encrypted, host);
    tcpServer->consoleMode = true;
    tcpServer->responsePacket(packetReply);
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

void MainPacketReceiver::responsePacket(Packet packetToSend)
{
    packetReply = packetToSend;

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
