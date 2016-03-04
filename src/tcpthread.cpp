/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */

#include "tcpthread.h"
#include "globals.h"
#include "packet.h"
#include <QDataStream>
#include <QDebug>
#include <QSettings>
#include <QDesktopServices>
#include <QDir>

TCPThread::TCPThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{

    init();
    sendFlag = false;
    incomingPersistent = false;
    sendPacket.clear();
    sendPacketPersistent.clear();


}

TCPThread::TCPThread(Packet sendPacket, QObject *parent)
    : QThread(parent), sendPacket(sendPacket)
{
    sendFlag = true;
    incomingPersistent = false;


}

void TCPThread::sendAnother(Packet sendPacket)
{

    QDEBUG() << "Send another packet to " << sendPacket.port;
    this->sendPacket = sendPacket;

}

void TCPThread::init() {

}


void TCPThread::wasdisconnected() {

    QDEBUG();
}

void TCPThread::writeResponse(QTcpSocket *sock, Packet tcpPacket) {

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    bool sendResponse = settings.value("sendReponse", false).toBool();
    bool sendSmartResponse = settings.value("smartResponseEnableCheck", false).toBool();
    QList<SmartResponseConfig> smartList;
    smartList.clear();
    smartList.append(Packet::fetchSmartConfig(1, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(2, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(3, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(4, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(5, SETTINGSFILE));



    QString responseData = (settings.value("responseHex","")).toString();
    int ipMode = settings.value("ipMode", 4).toInt();

    QByteArray smartData;
    smartData.clear();

    if(sendSmartResponse) {
        smartData = Packet::smartResponseMatch(smartList, tcpPacket.getByteArray());
    }


    if(sendResponse || !smartData.isEmpty())
    {
        Packet tcpPacketreply;
        tcpPacketreply.timestamp = QDateTime::currentDateTime();
        tcpPacketreply.name = "Reply to " + tcpPacket.timestamp.toString(DATETIMEFORMAT);
        tcpPacketreply.tcpOrUdp = "TCP";
        tcpPacketreply.fromIP = "You (Response)";
        if(ipMode < 6) {
            tcpPacketreply.toIP = Packet::removeIPv6Mapping(sock->peerAddress());
        } else {
            tcpPacketreply.toIP = (sock->peerAddress()).toString();
        }
        tcpPacketreply.port = sock->peerPort();
        tcpPacketreply.fromPort = sock->localPort();
        QByteArray data = Packet::HEXtoByteArray(responseData);
        tcpPacketreply.hexString = Packet::byteArrayToHex(data);

        QString testMacro = Packet::macroSwap(tcpPacketreply.asciiString());
        tcpPacketreply.hexString = Packet::ASCIITohex(testMacro);

        if(!smartData.isEmpty()) {
            tcpPacketreply.hexString = Packet::byteArrayToHex(smartData);
        }
        sock->write(tcpPacketreply.getByteArray());
        sock->waitForBytesWritten(2000);
        QDEBUG() << "packetSent " << tcpPacketreply.name << tcpPacketreply.hexString;
        emit packetSent(tcpPacketreply);

    }

}


void TCPThread::persistentConnectionLoop()
{
    QDEBUG() <<"Entering the forever loop";
    int ipMode = 4;
    QHostAddress theAddress(sendPacket.toIP);
    if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
        ipMode = 6;
    }

    int count = 0;
    while(clientConnection->state() == QAbstractSocket::ConnectedState && !closeRequest) {


        if(sendPacket.hexString.isEmpty() && sendPacket.persistent && (clientConnection->bytesAvailable() == 0)) {
            count++;
            if(count % 10 == 0) {
                QDEBUG() << "Loop and wait." << count++;
                emit connectStatus("Connected and idle.");
            }
            clientConnection->waitForReadyRead(200);
            if(!sendPacketPersistent.hexString.isEmpty()) {
                sendPacket.hexString = sendPacketPersistent.hexString;
                sendPacket.fromIP = "You";
                sendPacket.receiveBeforeSend = false;
                QDEBUGVAR(sendPacket.asciiString());
                QDEBUGVAR(sendPacket.hexString);
                sendPacketPersistent.clear();
            }
            continue;
        }

        if(clientConnection->state() != QAbstractSocket::ConnectedState && sendPacket.persistent) {
            QDEBUG() << "Connection broken.";
            emit connectStatus("Connection broken");

            break;
        }

        if(sendPacket.receiveBeforeSend) {
            QDEBUG() << "Wait for data before sending...";
            emit connectStatus("Waiting for data");
            clientConnection->waitForReadyRead(500);

            Packet tcpRCVPacket;
            tcpRCVPacket.hexString = Packet::byteArrayToHex(clientConnection->readAll());
            if(!tcpRCVPacket.hexString.trimmed().isEmpty()) {
                QDEBUG() << "Received: " << tcpRCVPacket.hexString;
                emit connectStatus("Received " + QString::number((tcpRCVPacket.hexString.size() / 3) + 1));

                tcpRCVPacket.timestamp = QDateTime::currentDateTime();
                tcpRCVPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
                tcpRCVPacket.tcpOrUdp = "TCP";

                if(ipMode < 6) {
                    tcpRCVPacket.fromIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());
                } else {
                    tcpRCVPacket.fromIP = (clientConnection->peerAddress()).toString();
                }


                QDEBUGVAR(tcpRCVPacket.fromIP);
                tcpRCVPacket.toIP = "You";
                tcpRCVPacket.port = sendPacket.fromPort;
                tcpRCVPacket.fromPort =    clientConnection->peerPort();
                emit packetSent(tcpRCVPacket);

            } else {
                QDEBUG() << "No pre-emptive receive data";
            }

        } // end receive before send


        //sendPacket.fromPort = clientConnection->localPort();
        emit connectStatus("Sending data:" + sendPacket.asciiString());
        QDEBUG() << "Attempting write data";
        clientConnection->write(sendPacket.getByteArray());
        emit packetSent(sendPacket);

        Packet tcpPacket;
        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        tcpPacket.tcpOrUdp = "TCP";
        if(ipMode < 6) {
            tcpPacket.fromIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());

        } else {
            tcpPacket.fromIP = (clientConnection->peerAddress()).toString();

        }
        QDEBUGVAR(tcpPacket.fromIP);

        tcpPacket.toIP = "You";
        tcpPacket.port = sendPacket.fromPort;
        tcpPacket.fromPort =    clientConnection->peerPort();

        clientConnection->waitForReadyRead(500);
        emit connectStatus("Waiting to receive");
        tcpPacket.hexString.clear();

        while(clientConnection->bytesAvailable()) {
            tcpPacket.hexString.append(" ");
            tcpPacket.hexString.append(Packet::byteArrayToHex(clientConnection->readAll()));
            tcpPacket.hexString = tcpPacket.hexString.simplified();
            clientConnection->waitForReadyRead(100);
        }


        if(!sendPacket.persistent) {
            emit connectStatus("Disconnecting");
            clientConnection->disconnectFromHost();
        }

        QDEBUG() << "packetSent " << tcpPacket.name << tcpPacket.asciiString();

        if(sendPacket.receiveBeforeSend) {
            if(!tcpPacket.hexString.isEmpty()) {
                emit packetSent(tcpPacket);
            }
        } else {
            emit packetSent(tcpPacket);
        }



        emit connectStatus("Reading response");
        tcpPacket.hexString  = clientConnection->readAll();

        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        emit packetSent(tcpPacket);


        if(!sendPacket.persistent) {
            break;
        } else {
            sendPacket.clear();
            sendPacket.persistent = true;
            QDEBUG() << "Persistent connection. Loop and wait.";
            continue;
        }
    } // end while connected

    if(closeRequest) {
        clientConnection->close();
        clientConnection->waitForDisconnected(300);
    }

}


void TCPThread::closeConnection()
{
    QDEBUG() << "Closing connection";
    clientConnection->close();
}


void TCPThread::run()
{
    closeRequest = false;

    //determine IP mode based on send address.
    int ipMode = 4;
    QHostAddress theAddress(sendPacket.toIP);
    if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
        ipMode = 6;
    }

    if(sendFlag) {
        QDEBUG() << "We are threaded sending!";
        clientConnection = new QTcpSocket(this);

        sendPacket.fromIP = "You";
        sendPacket.timestamp = QDateTime::currentDateTime();
        sendPacket.name = sendPacket.timestamp.toString(DATETIMEFORMAT);
        bool portpass = false;

        portpass = clientConnection->bind(); //use random port.
        if(portpass) {
            sendPacket.fromPort = clientConnection->localPort();
        }


        if(ipMode > 4) {
            clientConnection->connectToHost(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, QAbstractSocket::IPv6Protocol);

        } else {
            clientConnection->connectToHost(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);

        }
        clientConnection->waitForConnected(5000);


        if(sendPacket.delayAfterConnect > 0) {
            QDEBUG() << "sleeping " << sendPacket.delayAfterConnect;
            QObject().thread()->usleep(1000*sendPacket.delayAfterConnect);
        }

        QDEBUGVAR(clientConnection->localPort());

        if(clientConnection->state() == QAbstractSocket::ConnectedState)
        {
            emit connectStatus("Connected");
            sendPacket.port = clientConnection->peerPort();
            sendPacket.fromPort = clientConnection->localPort();

            persistentConnectionLoop();

            emit connectStatus("Not connected.");
            QDEBUG() << "Not connected.";

        } else {


            //qintptr sock = clientConnection->socketDescriptor();

            //sendPacket.fromPort = clientConnection->localPort();
            emit connectStatus("Could not connect.");
            QDEBUG() << "Could not connect";
            sendPacket.errorString = "Could not connect";
            emit packetSent(sendPacket);

        }

        QDEBUG() << "packetSent " << sendPacket.name;
        if(clientConnection->state() == QAbstractSocket::ConnectedState) {
            clientConnection->disconnectFromHost();
            clientConnection->waitForDisconnected(1000);
            emit connectStatus("Disconnected.");

        }
        clientConnection->close();
        clientConnection->deleteLater();

        return;
    }


    QTcpSocket sock;
    sock.setSocketDescriptor(socketDescriptor);

    connect(&sock, SIGNAL(disconnected()),
            this, SLOT(wasdisconnected()));

    //connect(&sock, SIGNAL(readyRead())

    Packet tcpPacket;
    QByteArray data;

    data.clear();
    tcpPacket.timestamp = QDateTime::currentDateTime();
    tcpPacket.name = tcpPacket.timestamp.toString(DATETIMEFORMAT);
    tcpPacket.tcpOrUdp = "TCP";

    if(ipMode < 6) {
        tcpPacket.fromIP = Packet::removeIPv6Mapping(sock.peerAddress());
    } else {
        tcpPacket.fromIP = (sock.peerAddress()).toString();
    }

    tcpPacket.toIP = "You";
    tcpPacket.port = sock.localPort();
    tcpPacket.fromPort = sock.peerPort();

    sock.waitForReadyRead(5000); //initial packet
    data = sock.readAll();
    tcpPacket.hexString = Packet::byteArrayToHex(data);
    emit packetSent(tcpPacket);
    writeResponse(&sock, tcpPacket);



    if(incomingPersistent) {
        clientConnection = &sock;
        QDEBUG() << "We are persistent incoming";
        sendPacket =  tcpPacket;
        sendPacket.persistent = true;
        sendPacket.hexString.clear();
        persistentConnectionLoop();
    }



/*

    QDateTime twentyseconds = QDateTime::currentDateTime().addSecs(30);

    while ( sock.bytesAvailable() < 1 && twentyseconds > QDateTime::currentDateTime()) {
        sock.waitForReadyRead();
        data = sock.readAll();
        tcpPacket.hexString = Packet::byteArrayToHex(data);
        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = tcpPacket.timestamp.toString(DATETIMEFORMAT);
        emit packetSent(tcpPacket);

        writeResponse(&sock, tcpPacket);
    }
*/
    sock.disconnectFromHost();
    sock.close();
}

void TCPThread::sendPersistant(Packet sendpacket)
{
    sendPacketPersistent = sendpacket;
}
