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
#include <QMessageBox>
#include <QHostInfo>
#include <QtGlobal>
#include "settings.h"

#include "persistentconnection.h"

PacketNetwork::PacketNetwork(QWidget *parent) :
    QObject(parent)
{

    joinedMulticast.clear();
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

    QApplication::processEvents();

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


    tcpServers.clear();
    udpServers.clear();
    sslServers.clear();
    receiveBeforeSend = false;
    delayAfterConnect = 0;
    tcpthreadList.clear();
    pcList.clear();

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QList<int> udpPortList, tcpPortList, sslPortList;
    int udpPort, tcpPort, sslPort;

    udpPortList = Settings::portsToIntList(settings.value("udpPort", "0").toString());
    tcpPortList = Settings::portsToIntList(settings.value("tcpPort", "0").toString());
    sslPortList = Settings::portsToIntList(settings.value("sslPort", "0").toString());

    QString ipMode = settings.value("ipMode", QHostAddress::AnyIPv4).toString();
    QDEBUGVAR(ipMode);

    QMessageBox msgBoxBindError;
    msgBoxBindError.setWindowTitle("Port bind error.");
    msgBoxBindError.setStandardButtons(QMessageBox::Ok);
    msgBoxBindError.setDefaultButton(QMessageBox::Ok);
    msgBoxBindError.setIcon(QMessageBox::Warning);
    const QString lowPortText = "Packet Sender attempted (and failed) to bind to a UDP port [PORT], which is less than 1024. \n\nPrivileged ports requires running Packet Sender with admin-level / root permissions.";
    const QString portConsumedText = "Packet Sender attempted (and failed) to bind to a UDP port [PORT].\n\n - Are you running multiple instances? \n\n - Trying to bind to a missing custom IP?";



    QUdpSocket *udpSocket;
    ThreadedTCPServer *ssl, *tcp;

    foreach (udpPort, udpPortList) {


        udpSocket = new QUdpSocket(this);

        bool bindResult = udpSocket->bind(
                              IPV4_OR_IPV6
                              , udpPort);

        if ((!bindResult) && (!erroronce)) {
            QDEBUGVAR(udpPort);
            erroronce = true;
            if (udpPort < 1024 && udpPort > 0) {
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


        QDEBUG() << connect(ssl, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(ssl, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(ssl, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));


    }





    sendResponse = settings.value("sendReponse", false).toBool();
    responseData = (settings.value("responseHex", "")).toString();
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


    if (settings.value("delayAfterConnectCheck", false).toBool()) {
        delayAfterConnect = 500;
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

    if(translateMacroSend) {
        QString data = Packet::macroSwap(sendpacket.asciiString());
        sendpacket.hexString = Packet::ASCIITohex(data);
    }

    if (sendpacket.persistent && (sendpacket.isTCP())) {
        //spawn a window.
        PersistentConnection * pcWindow = new PersistentConnection();
        pcWindow->sendPacket = sendpacket;
        pcWindow->init();


        QDEBUG() << connect(pcWindow->thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(pcWindow->thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(pcWindow->thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));
        QDEBUG() << connect(pcWindow->thread, SIGNAL(destroyed()), this, SLOT(disconnected()));


        pcWindow->show();

        //Prevent Qt from auto-destroying these windows.
        //TODO: Use a real connection manager.
        pcList.append(pcWindow);

        //TODO: Use a real connection manager.
        //prevent Qt from auto-destorying this thread while it tries to close.
        tcpthreadList.append(pcWindow->thread);

        return;

    }

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


    QApplication::processEvents();

    sendpacket.fromIP = "You";
    sendpacket.timestamp = QDateTime::currentDateTime();
    sendpacket.name = sendpacket.timestamp.toString(DATETIMEFORMAT);

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
