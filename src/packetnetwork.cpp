/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */

#include "packetnetwork.h"
#include "globals.h"
#include "settings.h"
#include<qstring.h>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>
#include <QSettings>
#include <QHostInfo>
#include <QtGlobal>
#include <QUrlQuery>
#include <QStandardPaths>
#include <windows.h>
#include <iostream>
#include "association.h"
#include <QStringList>
#include "dtlsthread.h"



#ifdef CONSOLE_BUILD
class QMessageBox {
public:
    static const int Ok = 0;
    static const int Warning = 0;
    void setWindowTitle(QString dummy){
        Q_UNUSED(dummy);
    }
    void setStandardButtons(int dummy) {
        Q_UNUSED(dummy);

    }
    void setDefaultButton(int dummy) {
        Q_UNUSED(dummy);

    }
    void setIcon(int dummy) {
        Q_UNUSED(dummy);

    }

    void setText(QString dummy) {
        Q_UNUSED(dummy);

    }

    void exec() {

    }

};

#else
#include <QMessageBox>

#include "persistentconnection.h"

#ifndef RENDER_ONLY
#include "persistenthttp.h"
#endif
#endif


PacketNetwork::PacketNetwork(QObject *parent) :
    QObject(parent)
{

    joinedMulticast.clear();
    http = new QNetworkAccessManager(this);
    QDEBUG() << " http connect attempt:" << connect(http, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(httpFinished(QNetworkReply*)));
    consoleMode = false;
}


void PacketNetwork::kill()
{

    QDEBUG();

    joinedMulticast.clear();

    //Packet Sender now supports any number of clients.
    QUdpSocket * udp;
    foreach (udp, udpServers) {
        udp->close();
        udp->deleteLater();
    }
    udpServers.clear();

    ThreadedTCPServer * tcpS;
    foreach (tcpS, tcpServers) {
        tcpS->close();
        tcpS->deleteLater();
    }
    tcpServers.clear();

    ThreadedTCPServer * sslS;
    foreach (sslS, sslServers) {
        sslS->close();
        sslS->deleteLater();
    }
    sslServers.clear();

    QDEBUG();

    QCoreApplication::processEvents();

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

QString PacketNetwork::getIPmode()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString ipMode = settings.value("ipMode", "4").toString();

    QHostAddress iph = Packet::IPV4_IPV6_ANY(ipMode);

    if(iph == QHostAddress::AnyIPv4) {
        return "IPv4 Mode";
    }
    if(iph == QHostAddress::AnyIPv6) {
        return "IPv6 Mode";
    }

    return ipMode;
}

bool PacketNetwork::DTLSListening()
{
    QUdpSocket * dtls;
    QDEBUGVAR(dtlsServers.size());
    foreach(dtls, dtlsServers) {
        QDEBUGVAR(dtls->state());
        if(dtls->state() == QAbstractSocket::BoundState) {
            //if(udp->state() == QAbstractSocket::ConnectedState) {
            return true;
        }
    }
    return false;
}

bool PacketNetwork::UDPListening()
{
    QUdpSocket * udp;
    QDEBUGVAR(udpServers.size());
    foreach(udp, udpServers) {
        QDEBUGVAR(udp->state());
        if(udp->state() == QAbstractSocket::BoundState) {
        //if(udp->state() == QAbstractSocket::ConnectedState) {
            return true;
        }
    }
    return false;
}

bool PacketNetwork::TCPListening()
{
    ThreadedTCPServer * tcp;
    foreach(tcp, tcpServers) {
        QDEBUGVAR(tcp->isListening());
        if(tcp->isListening()) {
            return true;
        }
    }
    return false;
}

bool PacketNetwork::SSLListening()
{
    ThreadedTCPServer * tcp;
    foreach(tcp, sslServers) {
        if(tcp->isListening()) {
            return true;
        }
    }
    return false;

}

bool PacketNetwork::IPv6Enabled()
{
    return !IPv4Enabled();
}

bool PacketNetwork::IPv4Enabled()
{
    QString ipMode = getIPmode();
    if(ipMode == "4") {
        return true;
    }
    return (ipMode.contains("v4") || ipMode.contains("."));
}


void PacketNetwork::setIPmode(int mode)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);


    if (mode > 4) {
        QDEBUG() << "Saving IPv6";
        settings.setValue("ipMode", "6");
    } else {
        QDEBUG() << "Saving IPv4";
        settings.setValue("ipMode", "4");
    }

}


void PacketNetwork::init()
{

    static bool erroronce = false;

    dtlsServers.clear();
    tcpServers.clear();
    udpServers.clear();
    sslServers.clear();
    receiveBeforeSend = false;
    delayAfterConnect = 0;
    tcpthreadList.clear();
    pcList.clear();


    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QList<int> udpPortList, tcpPortList, sslPortList, dtlsPortList;
    int dtlsPort = 0;
    int udpPort = 0;
    int tcpPort = 0;
    int sslPort = 0;

    //settings.setValue("dtlsPort", "12346");
    dtlsPortList = Settings::portsToIntList(settings.value("dtlsPort", "0").toString());
    udpPortList = Settings::portsToIntList(settings.value("udpPort", "0").toString());
    tcpPortList = Settings::portsToIntList(settings.value("tcpPort", "0").toString());
    sslPortList = Settings::portsToIntList(settings.value("sslPort", "0").toString());

    QString ipMode = settings.value("ipMode", "0.0.0.0").toString();
    QDEBUGVAR(ipMode);
    QMessageBox msgBoxBindError;
    msgBoxBindError.setWindowTitle(tr("Port bind error."));
    msgBoxBindError.setStandardButtons(QMessageBox::Ok);
    msgBoxBindError.setDefaultButton(QMessageBox::Ok);
    msgBoxBindError.setIcon(QMessageBox::Warning);
    const QString lowPortText = tr("Packet Sender attempted (and failed) to bind to port [PORT], which is less than 1024. \n\nPrivileged ports requires running Packet Sender with admin-level / root permissions.");
    const QString portConsumedText = tr("Packet Sender attempted (and failed) to bind to port [PORT].\n\n - Are you running multiple instances? \n\n - Trying to bind to a missing custom IP?");

#ifdef RENDER_ONLY
    tcpPortList.clear();
    sslPortList.clear();
#endif


    QUdpSocket *udpSocket, *dtlsSocket;
    ThreadedTCPServer *ssl, *tcp;
    foreach (dtlsPort, dtlsPortList) {
        /////////////////////////////////after adding the DtlsServer class//////////////////
        //DtlsServer dtlsServer;
        //bool bindResult = dtlsServer.serverSocket.listen(IPV4_OR_IPV6, dtlsPort);

        dtlsSocket = new QUdpSocket(this);

        bool bindResult = dtlsSocket->bind(
            IPV4_OR_IPV6
            , dtlsPort);

        dtlsSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 128);

        if ((!bindResult) && (!erroronce)) {
            QDEBUGVAR(dtlsPort);
            erroronce = true;
            if ((dtlsPort < 1024) && (dtlsPort > 0)) {
                QString msgText = lowPortText;
                msgText.replace("[PORT]", QString::number(dtlsPort));
                msgBoxBindError.setText(msgText);
                msgBoxBindError.exec();

            } else {
                QString msgText = portConsumedText;
                msgText.replace("[PORT]", QString::number(dtlsPort));
                msgBoxBindError.setText(msgText);
                msgBoxBindError.exec();

            }
            dtlsSocket->close();
            dtlsSocket->deleteLater();

        }

        if(bindResult) {
            dtlsServers.append(dtlsSocket);
////////////////////////////////////////after adding the DtlsServer class//////////////////

            //dtlsServers.append(dtlsServer.serverSocket);
        }

    }
    reJoinMulticast();


    foreach (udpPort, udpPortList) {


        udpSocket = new QUdpSocket(this);

        bool bindResult = udpSocket->bind(
                              IPV4_OR_IPV6
                              , udpPort);

        udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 128);

        if ((!bindResult) && (!erroronce)) {
            QDEBUGVAR(udpPort);
            erroronce = true;
            if ((udpPort < 1024) && (udpPort > 0)) {
                QString msgText = lowPortText;
                msgText.replace("[PORT]", QString::number(udpPort));
                msgBoxBindError.setText(msgText);
                msgBoxBindError.exec();

            } else {
                QString msgText = portConsumedText;
                msgText.replace("[PORT]", QString::number(udpPort));
                msgBoxBindError.setText(msgText);
                msgBoxBindError.exec();

            }
            udpSocket->close();
            udpSocket->deleteLater();

        }

        if(bindResult) {
            udpServers.append(udpSocket);
        }

    }

    reJoinMulticast();

    foreach (tcpPort, tcpPortList) {

        tcp = new ThreadedTCPServer(this);
        tcp->init(tcpPort, false, ipMode);
        tcpServers.append(tcp);

    }

    foreach (sslPort, sslPortList) {

        ssl = new ThreadedTCPServer(this);
        ssl->init(sslPort, true, ipMode);
        sslServers.append(ssl);

    }

    foreach (tcp, allTCPServers()) {
        if(!tcp->isListening() && (!erroronce)) {
            erroronce = true;
            if(tcp->serverPort() < 1024 && tcp->serverPort() > 0) {

                QString msgText = lowPortText;
                msgText.replace("[PORT]", QString::number(udpPort));
                msgBoxBindError.setText(msgText);
                msgBoxBindError.exec();
            } else {
                QString msgText = portConsumedText;
                msgText.replace("[PORT]", QString::number(udpPort));
                msgBoxBindError.setText(msgText);
                msgBoxBindError.exec();

            }

        }


        QDEBUG() << connect(tcp, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(tcp, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(tcp, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));


    }





    sendResponse = settings.value("sendReponse", false).toBool();
    responseData = (settings.value("responseHex", "")).toString();
    activateDTLS = settings.value("dtlsServerEnable", true).toBool();
    activateUDP = settings.value("udpServerEnable", true).toBool();
    activateTCP = settings.value("tcpServerEnable", true).toBool();
    activateSSL = settings.value("sslServerEnable", true).toBool();
    receiveBeforeSend = settings.value("attemptReceiveCheck", false).toBool();
    persistentConnectCheck = settings.value("persistentConnectCheck", false).toBool();
    sendSmartResponse = settings.value("smartResponseEnableCheck", false).toBool();
    translateMacroSend = settings.value("translateMacroSendCheck", true).toBool();

    smartList.clear();
    smartList.append(Packet::fetchSmartConfig(1, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(2, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(3, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(4, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(5, SETTINGSFILE));


#ifdef RENDER_ONLY
    activateTCP = false;
    activateSSL = false;
    smartList.clear();
#endif
    if (settings.value("delayAfterConnectCheck", false).toBool()) {
        delayAfterConnect = 500;
    }

    if (activateDTLS) {
        foreach (dtlsSocket, dtlsServers) {
/////////////////////////////////after adding the DtlsServer class//////////////////
//
//            connect(&server, &DtlsServer::errorMessage, this, &MainWindow::addErrorMessage);
//            connect(&server, &DtlsServer::warningMessage, this, &MainWindow::addWarningMessage);
//            connect(&server, &DtlsServer::infoMessage, this, &MainWindow::addInfoMessage);
//            connect(&server, &DtlsServer::datagramReceived, this, &MainWindow::addClientMessage);
//
            QDEBUG() << "signal/slot datagram connect: " << connect(dtlsSocket, SIGNAL(readyRead()),
                                                                    this, SLOT(readPendingDatagrams()));
        }

    } else {
        QDEBUG() << "udp server disable";
        foreach (udpSocket, udpServers) {
            udpSocket->close();
        }
        udpServers.clear();

    }



    if (activateUDP) {
        foreach (udpSocket, udpServers) {
            QDEBUG() << "signal/slot datagram connect: " << connect(udpSocket, SIGNAL(readyRead()),
                     this, SLOT(readPendingDatagrams()));
        }

    } else {
        QDEBUG() << "udp server disable";
        foreach (udpSocket, udpServers) {
            udpSocket->close();
        }
        udpServers.clear();

    }

    if (activateSSL) {


    } else {
        QDEBUG() << "ssl server disable";
        foreach (tcp, sslServers) {
            tcp->close();
        }
        sslServers.clear();

    }

    if (activateTCP) {


    } else {
        QDEBUG() << "tcp server disable";
        foreach (tcp, tcpServers) {
            tcp->close();
        }
        tcpServers.clear();
    }

}

//TODO add timed event feature?

QList<int> PacketNetwork::getDTLSPortsBound()
{
    QList<int> pList;
    pList.clear();
    QUdpSocket * dtls;
    foreach (dtls, dtlsServers) {
        if(dtls->BoundState == QAbstractSocket::BoundState) {
            if(dtls->localAddress().isMulticast()) {
                QDEBUG() << "This udp address is multicast";
            }
            pList.append(dtls->localPort());
        }
    }
    return pList;

}

QString PacketNetwork::getDTLSPortString()
{

    return Settings::intListToPorts(getDTLSPortsBound());
}

QList<int> PacketNetwork::getUDPPortsBound()
{
    QList<int> pList;
    pList.clear();
    QUdpSocket * udp;
    foreach (udp, udpServers) {
        if(udp->BoundState == QAbstractSocket::BoundState) {
            if(udp->localAddress().isMulticast()) {
                QDEBUG() << "This udp address is multicast";
            }
            pList.append(udp->localPort());
        }
    }
    return pList;

}

QString PacketNetwork::getUDPPortString()
{

    return Settings::intListToPorts(getUDPPortsBound());
}




QList<int> PacketNetwork::getTCPPortsBound()
{
    QList<int> pList;
    pList.clear();
    ThreadedTCPServer * tcp;
    foreach (tcp, tcpServers) {
        if(tcp->isListening()) {
            pList.append(tcp->serverPort());
        }
    }
    return pList;

}
QString PacketNetwork::getTCPPortString()
{
    return Settings::intListToPorts(getTCPPortsBound());
}





QList<int> PacketNetwork::getSSLPortsBound()
{

    QList<int> pList;
    pList.clear();
    ThreadedTCPServer * tcp;
    foreach (tcp, sslServers) {
        if(tcp->isListening()) {
            pList.append(tcp->serverPort());
        }
    }
    return pList;
}


QUdpSocket * PacketNetwork::findMulticast(QString multicast)
{
    QUdpSocket *udp = nullptr;

    if(joinedMulticast.contains(multicast)) {
        if(udpServers.size() > 0) {
            udp = this->udpServers.first();
        }
    }

    return udp;

}

QStringList PacketNetwork::multicastStringList()
{
    return joinedMulticast;
}

void PacketNetwork::reJoinMulticast()
{
    return; // this code does not work...

    if(joinedMulticast.isEmpty()) return;
    if(udpServers.isEmpty()) return;
    QUdpSocket *udp = udpServers.first();
    if(udp == nullptr) return;

    QString multicast;

    if(udp->state() == QAbstractSocket::BoundState) {
        foreach (multicast, joinedMulticast) {
            QDEBUG() << "rejoin" << multicast << udp->localPort();
            if(!udp->joinMulticastGroup(QHostAddress(multicast))) {
                QDEBUG() << udp->errorString();
            }
        }
    }
}



void PacketNetwork::leaveMulticast()
{
    QUdpSocket *udp;
    QString multicast;

    foreach (multicast, joinedMulticast) {
        foreach (udp, udpServers) {
            QDEBUG() << "Leaving" << multicast << udp->leaveMulticastGroup(QHostAddress(multicast));
        }
    }
    joinedMulticast.clear();
}

void PacketNetwork::joinMulticast(QString address)
{
    if(udpServers.isEmpty()) return;
    QUdpSocket *udp = udpServers.first();
    if(udp == nullptr) return;

    if(udp->state() == QAbstractSocket::BoundState) {
        QDEBUG() << "Joining " << address << ":" << udp->localPort() <<
        udp->joinMulticastGroup(QHostAddress(address));
        joinedMulticast << address;
        joinedMulticast.removeDuplicates();
    }

    QDEBUGVAR(joinedMulticast);



}


bool PacketNetwork::canSendMulticast(QString address)
{

    return joinedMulticast.contains(address);

}



QString PacketNetwork::getSSLPortString()
{
    return Settings::intListToPorts(getSSLPortsBound());
}



void PacketNetwork::readPendingDatagrams()
{

    QUdpSocket* udpSocket;

    //QDEBUG() << " got a datagram";
    bool once = false;
    bool isIPv6  = IPv6Enabled();

    foreach (udpSocket, udpServers) {

        if(udpSocket->state() == QAbstractSocket::UnconnectedState) {
            continue;
        }

        while (udpSocket->hasPendingDatagrams()) {

            if(!once) {
                isIPv6  = IPv6Enabled();
                once = true;
            }


            QHostAddress sender;
            int senderPort;

            QNetworkDatagram theDatagram = udpSocket->receiveDatagram(10000000);
            QByteArray datagram = theDatagram.data();
            sender =  theDatagram.senderAddress();
            senderPort = theDatagram.senderPort();

            QDEBUG() << "data size is" << datagram.size();
    //        QDEBUG() << debugQByteArray(datagram);

            Packet udpPacket;
            udpPacket.timestamp = QDateTime::currentDateTime();
            udpPacket.name = udpPacket.timestamp.toString(DATETIMEFORMAT);
            udpPacket.tcpOrUdp = "UDP";
            if (isIPv6) {
                udpPacket.fromIP = Packet::removeIPv6Mapping(sender);
            } else {
                udpPacket.fromIP = (sender).toString();
            }

            udpPacket.toIP = "You";

            if(theDatagram.destinationAddress().isMulticast()) {
                udpPacket.toIP = theDatagram.destinationAddress().toString();
            }


            udpPacket.port = udpSocket->localPort();
            udpPacket.fromPort = senderPort;

            QDEBUGVAR(senderPort);
    //        QDEBUG() << "sender port is " << sender.;

            udpPacket.hexString = Packet::byteArrayToHex(datagram);
            emit packetSent(udpPacket);

            QByteArray smartData;
            smartData.clear();

            if (sendSmartResponse) {
                smartData = Packet::smartResponseMatch(smartList, udpPacket.getByteArray());
            }


            if (sendResponse || !smartData.isEmpty()) {
                udpPacket.timestamp = QDateTime::currentDateTime();
                udpPacket.name = udpPacket.timestamp.toString(DATETIMEFORMAT);
                udpPacket.tcpOrUdp = "UDP";
                udpPacket.fromIP = "You (Response)";
                if (isIPv6) {
                    udpPacket.toIP = Packet::removeIPv6Mapping(sender);

                } else {
                    udpPacket.toIP = (sender).toString();
                }
                udpPacket.port = senderPort;
                udpPacket.fromPort = udpSocket->localPort();
                udpPacket.hexString = responseData;
                QString testMacro = Packet::macroSwap(udpPacket.asciiString());
                udpPacket.hexString = Packet::ASCIITohex(testMacro);

                if (!smartData.isEmpty()) {
                    udpPacket.hexString = Packet::byteArrayToHex(smartData);
                }

                QHostAddress resolved = resolveDNS(udpPacket.toIP);

                udpSocket->writeDatagram(udpPacket.getByteArray(), resolved, senderPort);
                emit packetSent(udpPacket);

            }

            //analyze the packet here.
            //emit packet signal;

        }


    }

}


QString PacketNetwork::debugQByteArray(QByteArray debugArray)
{
    QString outString = "";
    for (int i = 0; i < debugArray.size(); i++) {
        if (debugArray.at(i) != 0) {
            outString = outString + "\n" + QString::number(i) + ", 0x" + QString::number((unsigned char)debugArray.at(i), 16);
        }
    }
    return outString;
}


void PacketNetwork::disconnected()
{

    QDEBUG() << "Socket was disconnected.";
}


QHostAddress PacketNetwork::resolveDNS(QString hostname)
{

    QHostAddress address(hostname);
    if (QAbstractSocket::IPv4Protocol == address.protocol()) {
        return address;
    }

    if (QAbstractSocket::IPv6Protocol == address.protocol()) {
        return address;
    }

    QHostInfo info = QHostInfo::fromName(hostname);
    if (info.error() != QHostInfo::NoError) {
        return QHostAddress();
    } else {

        return info.addresses().at(0);
    }
}


//Multicast addresses ranges from 224.0.0.0 to 239.255.255.255
//Multicast addresses in IPv6 use the prefix ff00::/8
bool PacketNetwork::isMulticast(QString ip)
{
    QHostAddress address(ip.trimmed());
    if (QAbstractSocket::IPv4Protocol == address.protocol()) {
        //valid address
        QDEBUG() <<"Valid IPv4 multicast?";
        return address.isMulticast();
    } else if (QAbstractSocket::IPv6Protocol == address.protocol()) {
        //valid address

        //am I supporting IPv6?
        QDEBUG() <<"Valid IPv6 multicast?";
        QDEBUG() <<"I am not supporting this yet";
        return false;
        //return address.isMulticast();
    }

    return false;
}


void PacketNetwork::packetToSend(Packet sendpacket)
{

    sendpacket.receiveBeforeSend = receiveBeforeSend;
    sendpacket.delayAfterConnect = delayAfterConnect;
    sendpacket.persistent = persistentConnectCheck;
    if(consoleMode) {
        sendpacket.persistent = false;
    }

    if(translateMacroSend) {
        QString data = Packet::macroSwap(sendpacket.asciiString());
        sendpacket.hexString = Packet::ASCIITohex(data);
    }

#ifndef CONSOLE_BUILD
    if (sendpacket.persistent && (sendpacket.isTCP())) {
        //spawn a window.
        PersistentConnection * pcWindow = new PersistentConnection();
        TCPThread * thread = new TCPThread(sendpacket, this);
        pcWindow->sendPacket = sendpacket;
        pcWindow->init();
        pcWindow->thread = thread;


        QDEBUG() << ": thread Connection attempt " <<
                 connect(pcWindow, SIGNAL(persistentPacketSend(Packet)), thread, SLOT(sendPersistant(Packet)))
                 << connect(pcWindow, SIGNAL(closeConnection()), thread, SLOT(closeConnection()))
                 << connect(thread, SIGNAL(connectStatus(QString)), pcWindow, SLOT(statusReceiver(QString)))
                 << connect(thread, SIGNAL(packetSent(Packet)), pcWindow, SLOT(packetSentSlot(Packet)));


        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));


        //connect(&packetNetwork, SIGNAL(packetSent(Packet)),
        //        this, SLOT(toTrafficLog(Packet)));

        pcWindow->show();
        thread->start();


        //Network manager will manage this thread so the UI window doesn't need to.
        tcpthreadList.append(thread);

        return;

    }
#endif

    QHostAddress address;
    address.setAddress(sendpacket.toIP);


    if (sendpacket.isTCP()) {
        QDEBUG() << "Send this packet:" << sendpacket.name;


        TCPThread *thread = new TCPThread(sendpacket, this);

        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));
        QDEBUG() << connect(thread, SIGNAL(destroyed()), this, SLOT(disconnected()));

        //Prevent Qt from auto-destroying these threads.
        //TODO: Develop a real thread manager.
        tcpthreadList.append(thread);
        thread->start();
        return;
    }


    QCoreApplication::processEvents();

    sendpacket.fromIP = "You";
    sendpacket.timestamp = QDateTime::currentDateTime();
    sendpacket.name = sendpacket.timestamp.toString(DATETIMEFORMAT);


    if(sendpacket.isDTLS()){
        Dtlsthread * thread = new Dtlsthread(sendpacket, this);
        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)))
                 << connect(thread, SIGNAL(destroyed()), this, SLOT(disconnected()));

        if(settings.value("leaveSessionOpen").toString() == "true"){
            PersistentConnection * pcWindow = new PersistentConnection();

            pcWindow->sendPacket = sendpacket;
            pcWindow->init();
            pcWindow->dthread = thread;

            QDEBUG() << ": thread Connection attempt "
                     << connect(pcWindow, SIGNAL(persistentPacketSend(Packet)), thread, SLOT(sendPersistant(Packet)))
                     << connect(pcWindow, SIGNAL(closeConnection()), thread, SLOT(closeConnection()))
                     << connect(thread, SIGNAL(connectStatus(QString)), pcWindow, SLOT(statusReceiver(QString)))
                     << connect(thread, SIGNAL(packetSent(Packet)), pcWindow, SLOT(packetSentSlot(Packet)));

            pcWindow->show();
            thread->start();

        }
        else{
            thread->start();
            QTimer* timer = new QTimer(this);
            thread->timer = timer;
            connect(timer, SIGNAL(timeout()), thread, SLOT(onTimeout()));
            timer->start(1000);
        }
        dtlsthreadList.append(thread);
    }


    if (sendpacket.isUDP()) {
        QUdpSocket * sendUDP;
        bool oneoff = false;
        if(!udpServers.isEmpty()) {
            sendUDP = udpServers.first();
        }   else {
            QDEBUG() << "No server. Create a one-off";
            sendUDP = new QUdpSocket(this);
            sendUDP->bind(0);
            oneoff = true;
        }

        if(sendUDP->state() == QAbstractSocket::BoundState) {

            sendpacket.fromPort = sendUDP->localPort();
            QDEBUG() << "Sending data to :" << sendpacket.toIP << ":" << sendpacket.port;

            QHostAddress resolved = resolveDNS(sendpacket.toIP);

            QDEBUG() << "result:" << sendUDP->writeDatagram(sendpacket.getByteArray(), resolved, sendpacket.port);
            emit packetSent(sendpacket);

        }

        if(oneoff) {
            sendUDP->waitForBytesWritten();
            sendUDP->close();
            sendUDP->deleteLater();
        }


    }
    if (sendpacket.isHTTP()) {
        QDEBUG() << "http request" << sendpacket.requestPath;


        // TODO: catch ssl errors!
        // connect(http, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(sslErrorsSlot(QNetworkReply*, const QList<QSslError> & )));


        QString url = "";
        QString portUrl = "";
        QString portUrlS = ":" + QString::number(sendpacket.port);

        if(sendpacket.isHTTPS()) {
            url = "https://";
            if(sendpacket.port != 443) {
                portUrl = portUrlS;
            }
        } else {
            url = "http://";
            if(sendpacket.port != 80) {
                portUrl = portUrlS;
            }
        }
        url += sendpacket.toIP + portUrl + sendpacket.requestPath;

        QHash<QString, QString> bonusHeaders = Settings::getRawHTTPHeaders(sendpacket.toIP);


        //QDEBUGVAR(sendpacket.toIP);
        //QDEBUGVAR(sendpacket.requestPath);
        QDEBUGVAR(url);
        //return;
        QNetworkRequest request = QNetworkRequest(QUrl(url));

        sendpacket.fromIP = "You";
        sendpacket.fromPort = 0;
        if(consoleMode) {
            sendpacket.persistent = false;
        }

        http->setProperty("persistent", sendpacket.persistent);
        foreach(QString key, bonusHeaders.keys()) {
            QDEBUG()<<"Setting header" << key << bonusHeaders[key];
            request.setRawHeader(key.toLatin1(), bonusHeaders[key].toLatin1());
        }

        if(translateMacroSend) {
            QString data = Packet::macroSwap(sendpacket.asciiString());
            sendpacket.hexString = Packet::ASCIITohex(data);
        }

        QByteArray bytes = sendpacket.getByteArray();
        QString bytes_trimmed = QString(bytes.trimmed());

        if(sendpacket.isPOST()) {
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                "application/x-www-form-urlencoded");

            if(Settings::detectJSON_XML()) {

                if ((bytes_trimmed.startsWith("{") && bytes_trimmed.endsWith("}")) ||
                    (bytes_trimmed.startsWith("[") && bytes_trimmed.endsWith("]")) )  {
                    request.setHeader(QNetworkRequest::ContentTypeHeader,
                        "application/json");
                }


                if (bytes_trimmed.startsWith("<") && bytes_trimmed.endsWith(">")) {
                    request.setHeader(QNetworkRequest::ContentTypeHeader,
                        "application/xml");
                }
            }


            QDEBUG() << "http post request";
            http->post(request, sendpacket.getByteArray());

        } else {
            QDEBUG() << "http get request";
            http->get(request);
        }

        emit packetSent(sendpacket);

    }

}


void PacketNetwork::httpError(QNetworkRequest* pReply)
{
    QDEBUGVAR(pReply);

}

void PacketNetwork::sslErrorsSlot(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Packet errorPacket;
    errorPacket.init();

    QUrl url = reply->url();

    int defaultPort = 80;
    if(url.scheme().toLower().contains("https")) {
        defaultPort = 443;
    }

    errorPacket.fromIP = url.host();
    errorPacket.port = 0;
    errorPacket.fromPort = static_cast<unsigned int>(url.port(defaultPort));
    errorPacket.toIP = "You";
    errorPacket.tcpOrUdp = "HTTP";

    QDEBUGVAR(errors.size());
    if (errors.size() > 0) {

        QSslError sError;
        foreach (sError, errors) {
            errorPacket.hexString.clear();
            errorPacket.errorString = sError.errorString();
            emit packetSent(errorPacket);
        }

    }


    reply->ignoreSslErrors();
}

void PacketNetwork::httpFinished(QNetworkReply* pReply)
{

    QByteArray data = pReply->readAll();
    QString str = QString(data);
    str.truncate(1000);
    QDEBUG() << "finished http." << str;

    Packet httpPacket;
    httpPacket.init();

    QUrl url = pReply->url();

    QVariant status_code = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (status_code.isValid()) {
        // status_code.toInt();
        httpPacket.errorString = status_code.toString();
    }

    int defaultPort = 80;
    if(url.scheme().toLower().contains("https")) {
        defaultPort = 443;
    }
    httpPacket.fromIP = url.host();
    httpPacket.port = 0;
    httpPacket.fromPort = static_cast<unsigned int>(url.port(defaultPort));
    httpPacket.toIP = "You";
    httpPacket.hexString = Packet::byteArrayToHex(data);
    httpPacket.tcpOrUdp = "HTTP";

    if(pReply->error() != QNetworkReply::NoError) {
        QDEBUG() << "Ended in error";
        if(httpPacket.errorString.isEmpty()) {
            httpPacket.errorString = pReply->errorString();
        } else {
            httpPacket.errorString += ", " + pReply->errorString();
        }
    }

#ifdef RENDER_ONLY
    http->setProperty("persistent", false);
#endif

#ifndef RENDER_ONLY

#ifndef CONSOLE_BUILD
    QDEBUGVAR(http->property("persistent").toBool());
    if(http->property("persistent").toBool()) {
        PersistentHTTP * view = new PersistentHTTP();
        view->init(data, url);
        view->show();
        view->setAttribute(Qt::WA_DeleteOnClose);
    }
#endif
#endif
    emit packetSent(httpPacket);

}



QList<ThreadedTCPServer *> PacketNetwork::allTCPServers()
{
    QList<ThreadedTCPServer *> theServers;
    theServers.clear();
    ThreadedTCPServer * tcp;

    foreach (tcp, tcpServers) {
        theServers.append(tcp);
    }

    foreach (tcp, sslServers) {
        theServers.append(tcp);
    }

    return theServers;
}

std::vector<QString> PacketNetwork::getCmdInput(Packet sendpacket, QSettings& settings){
    //the array of cmdComponents: dataStr, toIp, toPort, sslPrivateKeyPath, sslLocalCertificatePath, sslCaFullPath
    std::vector<QString> cmdComponents;

    //get the data of the packet
    cmdComponents.push_back(QString::fromUtf8(sendpacket.getByteArray()));
    cmdComponents.push_back(sendpacket.toIP);
    cmdComponents.push_back(QString::number(sendpacket.port));

    //get the pathes for verification from the settings
    cmdComponents.push_back(settings.value("sslPrivateKeyPath", "default").toString());
    cmdComponents.push_back(settings.value("sslLocalCertificatePath", "default").toString());
    QString sslCaPath = settings.value("sslCaPath", "default").toString();

    //get the full path to to ca-signed-cert.pem file
    QDir dir(sslCaPath);
    if (dir.exists()) {
        QStringList nameFilters;
        nameFilters << "*.pem";  // Filter for .txt files

        dir.setNameFilters(nameFilters);
        QStringList fileList = dir.entryList();

        if (!fileList.isEmpty()) {
            // Select the first file that matches the filter
            cmdComponents.push_back(dir.filePath(fileList.first()));
        } else {
            qDebug() << "No matching files found.";
        }
    } else {
        qDebug() << "Directory does not exist.";
    }
    cmdComponents.push_back(settings.value("cipher", "AES256-GCM-SHA384").toString());
    return cmdComponents;
}

//void PacketNetwork::addServerResponse(const QString &clientAddress, const QByteArray &datagram, const QByteArray &plainText, QHostAddress serverAddress, quint16 serverPort, quint16 userPort)
//{
//    //ned to fix the "to port" field
//    //find a way do reach the client data (maybe use the clientInfo) inorder to present the "ToAddress" and "To Port"  fields in the traffic log area
//    //QStringList clientIpAndPort = clientInfo.split(':', Qt::KeepEmptyParts);

//    Packet recPacket;
//    recPacket.init();
//    recPacket.fromIP = serverAddress.toString();
//    recPacket.fromPort = serverPort;
//    QString massageFromTheOtherPeer = QString::fromUtf8(plainText);
//    recPacket.hexString = massageFromTheOtherPeer;
//    recPacket.toIP = clientAddress;
//    recPacket.port = userPort;
//    recPacket.errorString = "none";
//    recPacket.tcpOrUdp = "DTLS";

//    emit packetReceived(recPacket);
//}

