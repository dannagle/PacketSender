
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
    DtlsAssociation *dtlsAssociation = new DtlsAssociation(ipAddressHost, port, sendpacket.fromIP, sendpacket);
    dtlsAssociation->newMassageToSend = true;
    dtlsAssociation->massageToSend = cmdComponents[0];
    dtlsAssociation->socket;
    sendpacket.fromPort = dtlsAssociation->socket.localPort();
    connect(dtlsAssociation, &DtlsAssociation::serverResponse, this, &Dtlsthread::addServerResponse);
    dtlsAssociation->setKeyCertAndCaCert(cmdComponents[3],cmdComponents[4], cmdComponents[5]);
    dtlsAssociation->setCipher(cmdComponents[6]);
    //dtlsAssociation->startHandshake();
    connect(dtlsAssociation, &DtlsAssociation::handShakeComplited,this, &Dtlsthread::writeMassage);
    dtlsAssociation->startHandshake();
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

    ///emit packetReceived(recPacket);
}

void Dtlsthread::writeMassage(Packet packetToSend, DtlsAssociation* dtlsAssociation){

    //emit handShakeComplited(packetToSend, this);
    const qint64 written = dtlsAssociation->crypto.writeDatagramEncrypted(&(dtlsAssociation->socket), dtlsAssociation->massageToSend.toLatin1());
    if (written <= 0) {
        //emit errorMessage(tr("%1: failed to send a ping - %2").arg(name, crypto.dtlsErrorString()));
        return;
    }
}


