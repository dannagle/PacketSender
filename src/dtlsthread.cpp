
#include "dtlsthread.h"
#include "packet.h"
#include "association.h"
#include "packetnetwork.h"
//#include "QSettings"


Dtlsthread::Dtlsthread(Packet sendpacket, QObject *parent)
    : QThread(parent), sendpacket(sendpacket)
{}

Dtlsthread::~Dtlsthread() {
    // Destructor implementation (can be empty for this example)
}


void Dtlsthread::run()
{
    handShakeDone = false;
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        sendpacket.errorString ="Can't open settings file.";
    }

    //the vector of cmdComponents contains: dataStr, toIp, toPort, sslPrivateKeyPath, sslLocalCertificatePath, sslCaFullPath, chosen cipher
    std::vector<QString> cmdComponents = getCmdInput(sendpacket, settings);
    //qdtls

    const QString ipAddress = cmdComponents[1];
    QHostAddress ipAddressHost;
    ipAddressHost.setAddress(ipAddress);
    quint16 port = cmdComponents[2].toUShort();
    //+QString::number(sendpacket.fromPort);
    //QString test = QString::number(sendpacket.fromPort);
    DtlsAssociation *dtlsAssociationP = new DtlsAssociation(ipAddressHost, port, sendpacket.fromIP, sendpacket);
    //dtlsAssociation->setProtocol(QSsl::DtlsV1_2);

    dtlsAssociationP->newMassageToSend = true;
    //dtlsAssociationP->massageToSend = cmdComponents[0];
    dtlsAssociationP->socket;
    sendpacket.fromPort = dtlsAssociationP->socket.localPort();
    //connect(dtlsAssociationP, &DtlsAssociation::serverResponse, this, &Dtlsthread::addServerResponse);
    connect(this, &Dtlsthread::serverResponse, this, &Dtlsthread::addServerResponse);
    connect(dtlsAssociationP, &DtlsAssociation::receivedDatagram, this, &Dtlsthread::receivedDatagram);
    PacketNetwork *parentNetwork = qobject_cast<PacketNetwork*>(parent());
    connect(this, SIGNAL(packetReceived(Packet)), parentNetwork,  SLOT(toTrafficLog(Packet)));
    //connect(this, SIGNAL(packetReceived(Packet)), this,  SLOT(addServerResponse(Packet)));
    //dtlsAssociation->setKeyCertAndCaCert(cmdComponents[3],cmdComponents[4], cmdComponents[5]);
    dtlsAssociationP->setCipher(cmdComponents[6]);
    dtlsAssociation = dtlsAssociationP;
    //dtlsAssociation->startHandshake();
    //connect(dtlsAssociation, &DtlsAssociation::handShakeComplited,this, &Dtlsthread::writeMassage);
    connect(dtlsAssociation, &DtlsAssociation::handShakeComplited,this, &Dtlsthread::handShakeComplited);

    //QEventLoop loop;
    dtlsAssociation->startHandshake();
//    while(!handShakeDone){
//        continue;
//    }
    writeMassage(sendpacket, dtlsAssociation);

    persistentConnectionLoop();

    connectStatus("Connected");
    //emit packetSent(sendpacket);

    //dtlsAssociation->socket.waitForReadyRead();
    //dtlsAssociation->crypto.resumeHandshake(&(dtlsAssociation->socket));
    //loop.exec();

    //dtlsAssociation->crypto.doHandshake(&(dtlsAssociation->socket));
}


std::vector<QString> Dtlsthread::getCmdInput(Packet sendpacket, QSettings& settings){
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

void Dtlsthread::addServerResponse(const QString &clientAddress, const QByteArray &datagram, const QByteArray &plainText, QHostAddress serverAddress, quint16 serverPort, quint16 userPort)
{
    //ned to fix the "to port" field
    //find a way do reach the client data (maybe use the clientInfo) inorder to present the "ToAddress" and "To Port"  fields in the traffic log area
    //QStringList clientIpAndPort = clientInfo.split(':', Qt::KeepEmptyParts);

    Packet recPacket;
    recPacket.init();
    recPacket.fromIP = serverAddress.toString();
    recPacket.fromPort = serverPort;
    QString massageFromTheOtherPeer = QString::fromUtf8(plainText);
    recPacket.hexString = massageFromTheOtherPeer;
    recPacket.toIP = clientAddress;
    recPacket.port = userPort;
    recPacket.errorString = "none";
    recPacket.tcpOrUdp = "DTLS";

    //emit packetReceived(recPacket);
}


void Dtlsthread::handShakeComplited(){
    handShakeDone = true;
}

void Dtlsthread::writeMassage(Packet packetToSend, DtlsAssociation* dtlsAssociation){

    //emit handShakeComplited(packetToSend, this);
    const qint64 written = dtlsAssociation->crypto.writeDatagramEncrypted(&(dtlsAssociation->socket), packetToSend.asciiString().toLatin1());
    if (written <= 0) {
        //emit errorMessage(tr("%1: failed to send a ping - %2").arg(name, crypto.dtlsErrorString()));
        return;
    }
    emit packetSent(packetToSend);
    //packetToSend.hexString = "";
    //dtlsAssociation->socket.waitForReadyRead();
    //addServerResponse()
}



void Dtlsthread::persistentConnectionLoop()
{
    QUdpSocket* clientConnection = &(dtlsAssociation->socket);
    QDEBUG() << "Entering the forever loop";
    int ipMode = 4;
    QHostAddress theAddress(sendpacket.toIP);
    if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
        ipMode = 6;
    }

    int count = 0;
    while (clientConnection->state() == QAbstractSocket::ConnectedState) {
        insidePersistent = true;


        if (sendpacket.hexString.isEmpty() /*&& sendpacket.persistent */ && (clientConnection->bytesAvailable() == 0)) {
            count++;
            if (count % 10 == 0) {
                //QDEBUG() << "Loop and wait." << count++ << clientConnection->state();
                emit connectStatus("Connected and idle.");
            }
            clientConnection->waitForReadyRead(200);
            continue;
        }

        if (clientConnection->state() != QAbstractSocket::ConnectedState /*&& sendPacket.persistent*/) {
            QDEBUG() << "Connection broken.";
            emit connectStatus("Connection broken");

            break;
        }

        if (sendpacket.receiveBeforeSend) {
            QDEBUG() << "Wait for data before sending...";
            emit connectStatus("Waiting for data");
            clientConnection->waitForReadyRead(500);

            Packet tcpRCVPacket;
            tcpRCVPacket.hexString = Packet::byteArrayToHex(clientConnection->readAll());
            if (!tcpRCVPacket.hexString.trimmed().isEmpty()) {
                QDEBUG() << "Received: " << tcpRCVPacket.hexString;
                emit connectStatus("Received " + QString::number((tcpRCVPacket.hexString.size() / 3) + 1));

                tcpRCVPacket.timestamp = QDateTime::currentDateTime();
                tcpRCVPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
                tcpRCVPacket.tcpOrUdp = "DTLS";

                if (ipMode < 6) {
                    tcpRCVPacket.fromIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());
                } else {
                    tcpRCVPacket.fromIP = (clientConnection->peerAddress()).toString();
                }


                QDEBUGVAR(tcpRCVPacket.fromIP);
                tcpRCVPacket.toIP = "You";
                tcpRCVPacket.port = sendpacket.fromPort;
                tcpRCVPacket.fromPort =    clientConnection->peerPort();
                if (tcpRCVPacket.hexString.size() > 0) {
                    emit packetSent(tcpRCVPacket);

                    // Do I need to reply?
                    writeMassage(tcpRCVPacket, dtlsAssociation);

                }

            } else {
                QDEBUG() << "No pre-emptive receive data";
            }

        } // end receive before send


        //sendPacket.fromPort = clientConnection->localPort();
        if(sendpacket.getByteArray().size() > 0) {
            emit connectStatus("Sending data:" + sendpacket.asciiString());
            QDEBUG() << "Attempting write data";
            //writeMassage(sendpacket, dtlsAssociation);
            //clientConnection->write(sendpacket.getByteArray());
            //emit packetSent(sendpacket);
        }

        Packet tcpPacket;
        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        tcpPacket.tcpOrUdp = "DTLS";


        if (ipMode < 6) {
            tcpPacket.fromIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());

        } else {
            tcpPacket.fromIP = (clientConnection->peerAddress()).toString();

        }
        QDEBUGVAR(tcpPacket.fromIP);

        tcpPacket.toIP = "You";
        tcpPacket.port = dtlsAssociation->socket.localPort();
        tcpPacket.fromPort = sendpacket.port;/////////////////////fromport

        clientConnection->waitForReadyRead(500);
        emit connectStatus("Waiting to receive");
        tcpPacket.hexString.clear();

        while (clientConnection->bytesAvailable()) {
            tcpPacket.hexString.append(" ");
            tcpPacket.hexString.append(Packet::byteArrayToHex(clientConnection->readAll()));
            tcpPacket.hexString = tcpPacket.hexString.simplified();
            clientConnection->waitForReadyRead(100);
        }


        //        if (!sendpacket.persistent) {
        //            emit connectStatus("Disconnecting");
        //            clientConnection->disconnectFromHost();
        //        }

        QDEBUG() << "packetSent " << tcpPacket.name << tcpPacket.hexString.size();

        if (sendpacket.receiveBeforeSend) {
            if (!tcpPacket.hexString.isEmpty()) {
                emit packetSent(tcpPacket);
            }
        } else {
            //emit packetSent(tcpPacket);
        }

        // Do I need to reply?
        //writeResponse(clientConnection, tcpPacket);
        //writeMassage(tcpPacket, dtlsAssociation);



        emit connectStatus("Reading response");
        //tcpPacket.hexString  = clientConnection->readAll();
        tcpPacket.hexString = recievedMassage;

        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);


        if (tcpPacket.hexString.size() > 0) {

            //emit packetSent(tcpPacket);

            // Do I need to reply?
            //writeMassage(tcpPacket, dtlsAssociation);
            //here we find out if there is new massage from server
            //emit packetReceived(tcpPacket);
            //emit connectStatus("last sent massage: " + recievedMassage);
            tcpPacket.hexString = "";
            recievedMassage = "";
            sendpacket.hexString = "";
        }



        //        if (!sendPacket.persistent) {
        //            break;
        //        } else {
        //            sendPacket.clear();
        //            sendPacket.persistent = true;
        //            QDEBUG() << "Persistent connection. Loop and wait.";
        //            continue;
        //        }
    } // end while connected

    //    if (closeRequest) {
    //        clientConnection->close();
    //        clientConnection->waitForDisconnected(100);
    //    }

}
void Dtlsthread::receivedDatagram(QByteArray plainText){
    recievedMassage = QString::fromUtf8(plainText);
    Packet recPacket;
    recPacket.init();
    recPacket.fromIP = dtlsAssociation->crypto.peerAddress().toString();
    recPacket.fromPort = dtlsAssociation->crypto.peerPort();
    QString massageFromTheOtherPeer = QString::fromUtf8(plainText);
    recPacket.hexString = massageFromTheOtherPeer;
    recPacket.toIP = dtlsAssociation->socket.localAddress().toString();
    recPacket.port = dtlsAssociation->socket.localPort();
    recPacket.errorString = "none";
    recPacket.tcpOrUdp = "DTLS";

    emit packetReceived(recPacket);
}


void Dtlsthread::sendPersistant(Packet sendpacket)
{
    QUdpSocket* clientConnection = &(dtlsAssociation->socket);

    if ((!sendpacket.hexString.isEmpty()) && (clientConnection->state() == QAbstractSocket::ConnectedState)) {
        QDEBUGVAR(sendpacket.hexString);
        sendpacket.fromPort = clientConnection->localPort();
        writeMassage(sendpacket,dtlsAssociation);

        sendpacket.fromIP = "You";

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        int ipMode = settings.value("ipMode", 4).toInt();


        if (ipMode < 6) {
            sendpacket.toIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());
        } else {
            sendpacket.toIP = (clientConnection->peerAddress()).toString();
        }

        sendpacket.port = clientConnection->peerPort();
        sendpacket.fromPort = clientConnection->localPort();
        sendpacket.tcpOrUdp = "DTLS";

        //emit packetSent(sendpacket);
    }
}
