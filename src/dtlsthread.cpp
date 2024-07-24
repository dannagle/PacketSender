
#include "dtlsthread.h"
#include "packet.h"
#include "association.h"
#include "packetnetwork.h"
#include "mainwindow.h"
#include <QThread>
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
    dtlsAssociation = initDtlsAssociation();
    dtlsAssociation->closeRequest = false;
    connect(dtlsAssociation, &DtlsAssociation::handShakeComplited,this, &Dtlsthread::handShakeComplited);

    dtlsAssociation->startHandshake();
    //dtlsAssociation->crypto.continueHandshake()
    connect(&(dtlsAssociation->crypto), &QDtls::handshakeTimeout, this, &Dtlsthread::onHandshakeTimeout);
    writeMassage(sendpacket, dtlsAssociation);
    dtlsAssociation->socket.waitForReadyRead();
    if(persistentRequest){
        persistentConnectionLoop();
        connectStatus("Connected");
    }
    else{
        return;
    }
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




void Dtlsthread::handShakeComplited(){
    handShakeDone = true;
}

void Dtlsthread::writeMassage(Packet packetToSend, DtlsAssociation* dtlsAssociation){
    const qint64 written = dtlsAssociation->crypto.writeDatagramEncrypted(&(dtlsAssociation->socket), packetToSend.asciiString().toLatin1());
    if (written <= 0) {
        packetToSend.errorString.append(" Failed to send");
        //if(dtlsAssociation->crypto.isConnectionEncrypted()){
            emit packetSent(packetToSend);

        //}
        return;
    }
    emit packetSent(packetToSend);
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
    while (clientConnection->state() == QAbstractSocket::ConnectedState && !(dtlsAssociation->closeRequest)) {
        insidePersistent = true;


        if (sendpacket.hexString.isEmpty() /*&& sendpacket.persistent */ && (clientConnection->bytesAvailable() == 0)) {
            count++;
            if (count % 10 == 0) {
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

        if(sendpacket.getByteArray().size() > 0) {
            emit connectStatus("Sending data:" + sendpacket.asciiString());
            QDEBUG() << "Attempting write data";
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
        tcpPacket.fromPort = sendpacket.port;

        clientConnection->waitForReadyRead(500);
        emit connectStatus("Waiting to receive");
        tcpPacket.hexString.clear();

        while (clientConnection->bytesAvailable()) {
            tcpPacket.hexString.append(" ");
            tcpPacket.hexString.append(Packet::byteArrayToHex(clientConnection->readAll()));
            tcpPacket.hexString = tcpPacket.hexString.simplified();
            clientConnection->waitForReadyRead(100);
        }

        QDEBUG() << "packetSent " << tcpPacket.name << tcpPacket.hexString.size();

        if (sendpacket.receiveBeforeSend) {
            if (!tcpPacket.hexString.isEmpty()) {
                emit packetSent(tcpPacket);
            }
        } else {
            //emit packetSent(tcpPacket);
        }

        emit connectStatus("Reading response");

        tcpPacket.hexString = recievedMassage;

        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);


        if (tcpPacket.hexString.size() > 0) {
            tcpPacket.hexString = "";
            recievedMassage = "";
            sendpacket.hexString = "";
        }

    } // end while connected
    emit connectStatus("Disconnected");

    if (dtlsAssociation->closeRequest) {
        emit connectStatus("Disconnected");
        clientConnection->close();
        clientConnection->waitForDisconnected(100);
        //quit();
    }

    insidePersistent = false;

}
void Dtlsthread::receivedDatagram(QByteArray plainText){
    respondRecieved = true;
    //MainWindow *parentNetwork = qobject_cast<MainWindow*>(parent());
    //connect(this, SIGNAL(packetReceived(Packet)), parentNetwork,  SLOT(toTrafficLog(Packet)));
    recievedMassage = QString::fromUtf8(plainText);
    Packet recPacket;
    recPacket.init();
    recPacket.fromIP = dtlsAssociation->crypto.peerAddress().toString();
    recPacket.fromPort = dtlsAssociation->crypto.peerPort();
    QString massageFromTheOtherPeer = QString::fromUtf8(plainText);
    recPacket.hexString = recPacket.ASCIITohex(massageFromTheOtherPeer);
    recPacket.toIP = dtlsAssociation->socket.localAddress().toString();
    recPacket.port = dtlsAssociation->socket.localPort();
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

    }
}

void Dtlsthread::onTimeout(){
    if(respondRecieved == false){
        dtlsAssociation->closeRequest = true;

//       if(!handShakeDone && retries < 5){//we can test handShakeDone for each thread because the server serving only one client at one time according to the udp socket
//           retries++;
//           dtlsAssociation->startHandshake();
//
//       }else{
            sendpacket.errorString = "Error timeout" + dtlsAssociation->packetToSend.errorString /*+ errors*/ ;
            //emit packetSent(sendpacket);
            handShakeDone = false;
            closeRequest = true;
            timer->stop();
//      }
    } else{
        closeRequest = true;
        timer->stop();
        this->exit();
    }

    //dtlsAssociation->closeRequest = true;
    //timer->stop();
    //&& leaveSessionOpen
    //emit packetSent(sendpacket);

    //if(!(dtlsAssociation->crypto.isConnectionEncrypted())){
    //    //QString  errors = dtlsAssociation->crypto.dtlsErrorString();
    //    sendpacket.errorString = "Error timeout" + dtlsAssociation->packetToSend.errorString /*+ errors*/ ;
    //    emit packetSent(sendpacket);
    //}
}

DtlsAssociation* Dtlsthread::initDtlsAssociation(){
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        sendpacket.errorString ="Can't open settings file.";
    }
    //the vector of cmdComponents contains: dataStr, toIp, toPort, sslPrivateKeyPath, sslLocalCertificatePath, sslCaFullPath, chosen cipher
    std::vector<QString> cmdComponents = getCmdInput(sendpacket, settings);
    const QString ipAddress = cmdComponents[1];
    QHostAddress ipAddressHost;
    ipAddressHost.setAddress(ipAddress);
    quint16 port = cmdComponents[2].toUShort();
    DtlsAssociation *dtlsAssociationP = new DtlsAssociation(ipAddressHost, port, sendpacket.fromIP, cmdComponents);
    sendpacket.fromPort = dtlsAssociationP->socket.localPort();
    connect(dtlsAssociationP, &DtlsAssociation::receivedDatagram, this, &Dtlsthread::receivedDatagram);
    PacketNetwork *parentNetwork = qobject_cast<PacketNetwork*>(parent());
    //connect(this, SIGNAL(packetReceived(Packet)), parentNetwork,  SLOT(toTrafficLog(Packet)));
    dtlsAssociationP->setCipher(settings.value("cipher", "cipher suit doesn't found").toString());

    return dtlsAssociationP;
}

void Dtlsthread::onHandshakeTimeout() {
    // Introduce a delay before retrying
    QTimer::singleShot(1000, this, &Dtlsthread::retryHandshake);
}

void Dtlsthread::retryHandshake() {
    dtlsAssociation->crypto.handleTimeout(&dtlsAssociation->socket);
}
