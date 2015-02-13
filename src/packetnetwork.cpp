/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */

#include "packetnetwork.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>
#include <QSettings>

PacketNetwork::PacketNetwork(QWidget *parent) :
    QTcpServer(parent)
{
}


void PacketNetwork::kill()
{
    udpSocket->close();
    tcpSocket->close();
    close();

    QApplication::processEvents();
    udpSocket->deleteLater();
    tcpSocket->deleteLater();
    QApplication::processEvents();

}

void PacketNetwork::incomingConnection(qintptr socketDescriptor)
{
    QDEBUG() << "new tcp connection";

    TCPThread *thread = new TCPThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
             << connect(thread, SIGNAL(toStatusBar(QString,int,bool)), this, SLOT(toStatusBarECHO(QString,int,bool)))
             << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));

    thread->start();

}

void PacketNetwork::packetReceivedECHO(Packet sendpacket)
{
    emit packetReceived(sendpacket);
}

void PacketNetwork::toStatusBarECHO(const QString &message, int timeout, bool override)
{
    emit toStatusBar(message, timeout, override);

}

void PacketNetwork::packetSentECHO(Packet sendpacket)
{
    emit packetSent(sendpacket);

}

void PacketNetwork::init()
{

    udpSocket = new QUdpSocket(this);
    tcpSocket = new QTcpSocket(this);

    receiveBeforeSend = false;
    delayAfterConnect = 0;

    tcpthreadList.clear();

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    int udpPort = settings.value("udpPort", 55056).toInt();

    qDebug() << __FILE__ << "/" <<__LINE__ << "udpSocket bind: " << udpSocket->bind(QHostAddress::Any, udpPort);

    int tcpPort = settings.value("tcpPort", 55056).toInt();

    qDebug() << __FILE__ << "/" <<__LINE__ << "tcpServer bind: " << listen(QHostAddress::Any, tcpPort);


    sendResponse = settings.value("sendReponse", false).toBool();
    responseData = (settings.value("responseHex","")).toString();
    activateUDP = settings.value("udpServerEnable", true).toBool();
    activateTCP = settings.value("tcpServerEnable", true).toBool();
    receiveBeforeSend = settings.value("attemptReceiveCheck", false).toBool();
    persistentConnectCheck = settings.value("persistentConnectCheck", false).toBool();

    if(settings.value("delayAfterConnectCheck", false).toBool()) {
        delayAfterConnect = 500;
    }



    if(activateUDP)
    {
        QDEBUG()<< "signal/slot datagram connect: " << connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));

    } else {
        QDEBUG() << "udp server disable";
    }


    if(activateTCP)
    {


    } else {
        QDEBUG() << "tcp server disable";
        close();
    }

}

//TODO add timed event feature?


int PacketNetwork::getUDPPort()
{
    if(activateUDP)
    {
        return udpSocket->localPort();
    } else {
        return 0;
    }
 }

int PacketNetwork::getTCPPort()
{
    if(isListening())
    {
        return serverPort();
    } else {
        return 0;
    }

}


void PacketNetwork::readPendingDatagrams()
{
    //QDEBUG() << " got a datagram";
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        QDEBUG() << "data size is"<<datagram.size();
//        QDEBUG() << debugQByteArray(datagram);

        Packet udpPacket;
        udpPacket.timestamp = QDateTime::currentDateTime();
        udpPacket.name = udpPacket.timestamp.toString(DATETIMEFORMAT);
        udpPacket.tcpOrUdp = "UDP";
        udpPacket.fromIP = sender.toString();
        udpPacket.toIP = "You";
        udpPacket.port = getUDPPort();
        udpPacket.fromPort = senderPort;

        QDEBUGVAR(senderPort);
//        QDEBUG() << "sender port is " << sender.;

        udpPacket.hexString = Packet::byteArrayToHex(datagram);
        emit packetSent(udpPacket);

        if(sendResponse)
        {
            udpPacket.timestamp = QDateTime::currentDateTime();
            udpPacket.name = udpPacket.timestamp.toString(DATETIMEFORMAT);
            udpPacket.tcpOrUdp = "UDP";
            udpPacket.fromIP = "You (Response)";
            udpPacket.toIP = sender.toString();
            udpPacket.port = senderPort;
            udpPacket.fromPort = getUDPPort();
            udpPacket.hexString = responseData;

            udpSocket->writeDatagram(udpPacket.getByteArray(),sender,senderPort);
            emit packetSent(udpPacket);

        }

        //analyze the packet here.
        //emit packet signal;

    }
}


QString PacketNetwork::debugQByteArray(QByteArray debugArray)
{
    QString outString = "";
    for(int i = 0; i < debugArray.size(); i++)
    {
        if(debugArray.at(i) != 0)
        {
            outString = outString + "\n" + QString::number(i) + ", 0x" + QString::number((unsigned char)debugArray.at(i), 16);
        }
    }
    return outString;
}


void PacketNetwork::disconnected()
{
    QDEBUG() << "Socket was disconnected.";
}

void PacketNetwork::packetToSend(Packet sendpacket)
{

    sendpacket.receiveBeforeSend = receiveBeforeSend;
    sendpacket.delayAfterConnect = delayAfterConnect;
    sendpacket.persistent = persistentConnectCheck;

    QString hashAddress = sendpacket.toIP + ":" + sendpacket.port;
    if(tcpthreadList[hashAddress] != NULL) {
        if(tcpthreadList[hashAddress]->isRunning()) {
            tcpthreadList[hashAddress]->sendAnother(sendpacket);
            return;
        }
    }

    QHostAddress address;
    address.setAddress(sendpacket.toIP);


    if(sendpacket.tcpOrUdp.toUpper() == "TCP")
    {
        QDEBUG() << "Send this packet:" << sendpacket.name;


        TCPThread *thread = new TCPThread(sendpacket, this);

        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString,int,bool)), this, SLOT(toStatusBarECHO(QString,int,bool)))
                 << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));
        QDEBUG() << connect(thread, SIGNAL(destroyed()),this, SLOT(disconnected()));

        tcpthreadList[hashAddress] = thread;
        thread->start();
        return;
    }


    QApplication::processEvents();

    sendpacket.fromIP = "You";
    sendpacket.timestamp = QDateTime::currentDateTime();
    sendpacket.name = sendpacket.timestamp.toString(DATETIMEFORMAT);

    if(sendpacket.tcpOrUdp.toUpper() == "UDP")
    {
        sendpacket.fromPort = getUDPPort();
        QDEBUG() << "Sending data to :" << sendpacket.toIP << ":" << sendpacket.port;
        QDEBUG() << "result:" << udpSocket->writeDatagram(sendpacket.getByteArray(), address, sendpacket.port);
        emit packetSent(sendpacket);
    }


/*
    else {

        sendpacket.fromPort = getTCPPort();

        tcpSocket->connectToHost(address,  sendpacket.port);
        tcpSocket->waitForConnected(5000);

        if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        {
            tcpSocket->write(sendpacket.getByteArray());

            Packet tcpPacket;
            tcpPacket.timestamp = QDateTime::currentDateTime();
            tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
            tcpPacket.tcpOrUdp = "TCP";
            tcpPacket.fromIP = tcpSocket->peerAddress().toString();
            tcpPacket.toIP = "You";
            tcpPacket.port = sendpacket.fromPort;
            tcpPacket.fromPort =    tcpSocket->peerPort();

            tcpSocket->waitForReadyRead(2000);
            tcpPacket.hexString = Packet::byteArrayToHex(tcpSocket->readAll());
            tcpSocket->disconnectFromHost();

            emit packetSent(tcpPacket);



            sendpacket.response = tcpSocket->readAll();
        } else {
                    QDEBUG() << "Could not connect";
                    sendpacket.errorString = "Could not connect";
        }

        tcpSocket->disconnectFromHost();

    }


    emit packetSent(sendpacket);

*/


}

/*
public slots:
    void packetReceivedECHO(Packet sendpacket);
    void toStatusBarECHO(const QString & message, int timeout = 0, bool override = false);
    void packetSentECHO(Packet sendpacket);
*/


void PacketNetwork::newSession()
{
    /*
    QDEBUG() <<"new TCP connection";

    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

    QByteArray data = clientConnection->readAll();
    QDEBUGVAR(QString(data));

    TCPThread *thread = new TCPThread(clientConnection->socketDescriptor(), this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
             << connect(thread, SIGNAL(toStatusBar(QString,int,bool)), this, SLOT(toStatusBarECHO(QString,int,bool)))
             << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));

    thread->start();
    */

    /*
    QByteArray data;
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    Packet tcpPacket;
    tcpPacket.timestamp = QDateTime::currentDateTime();
    tcpPacket.name = tcpPacket.timestamp.toString(DATETIMEFORMAT);
    tcpPacket.tcpOrUdp = "TCP";
    tcpPacket.fromIP = clientConnection->peerAddress().toString();
    tcpPacket.toIP = "You";
    tcpPacket.port = getTCPPort();
    tcpPacket.fromPort = clientConnection->peerPort();

    clientConnection->waitForReadyRead(2000);
    data = clientConnection->readAll();
    int loopCount = 0;
    while(data.size() < 10000 && clientConnection->isOpen()) {
        data.append(clientConnection->readAll());
        clientConnection->waitForReadyRead(200);
        loopCount++;
        if(loopCount > 10) {
            break;
        }
    }
    tcpPacket.hexString = Packet::byteArrayToHex(data);
    emit packetSent(tcpPacket);

    if(sendResponse)
    {
        Packet tcpPacketreply;
        tcpPacketreply.timestamp = QDateTime::currentDateTime();
        tcpPacketreply.name = "Reply to " + tcpPacket.timestamp.toString(DATETIMEFORMAT);
        tcpPacketreply.tcpOrUdp = "TCP";
        tcpPacketreply.fromIP = "You (Response)";
        tcpPacketreply.toIP = clientConnection->peerAddress().toString();
        tcpPacketreply.port = clientConnection->peerPort();
        tcpPacketreply.fromPort = getTCPPort();
        data = Packet::HEXtoByteArray(responseData);
        tcpPacketreply.hexString = Packet::byteArrayToHex(data);
        clientConnection->write(data);
        clientConnection->waitForBytesWritten(2000);
        emit packetSent(tcpPacketreply);

    }

    clientConnection->disconnectFromHost();
    clientConnection->close();

    return;
*/

}
