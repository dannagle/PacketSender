// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dtlsserver.h"
#include "globals.h"

#include <algorithm>
#ifndef CONSOLE_BUILD
#include <QMessageBox>

#else

class QMessageBox {
public:
    QMessageBox() {

    };
    ~QMessageBox() {

    }
    static void critical(void * ptr, QString msg1, QString msg2) {
        return;
    }

};
#endif


#if QT_VERSION > QT_VERSION_CHECK(6, 00, 0)


bool DtlsServer::closeNotifyReceived = false;

namespace {


QString peer_info(const QHostAddress &address, quint16 port)
{
    const static QString info = QStringLiteral("(%1:%2)");
    return info.arg(address.toString()).arg(port);
}


QString connection_info(QDtls *connection)
{
    QString info(DtlsServer::tr("Session cipher: "));
    info += connection->sessionCipher().name();

    info += DtlsServer::tr("; session protocol: ");
    switch (connection->sessionProtocol()) {
    case QSsl::DtlsV1_2:
        info += DtlsServer::tr("DTLS 1.2.");
        break;
    default:
        info += DtlsServer::tr("Unknown protocol.");
    }

    return info;
}

} // unnamed namespace


//! [1]
DtlsServer::DtlsServer()
{
    connect(&serverSocket, &QAbstractSocket::readyRead, this, &DtlsServer::readyRead);
    loadKeyLocalCertCaCert();
}
//! [1]

DtlsServer::~DtlsServer()
{
    shutdown();
}

//! [2]
bool DtlsServer::listen(const QHostAddress &address, quint16 port)
{
    if (address != serverSocket.localAddress() || port != serverSocket.localPort()) {
        shutdown();
        listening = serverSocket.bind(address, port);
        if (!listening)
            QMessageBox::critical(nullptr, "Bind Error", "The server can't bind " + QString::number(serverSocket.localPort()) + serverSocket.errorString());
    } else {
        listening = true;
    }

    return listening;
}
//! [2]

bool DtlsServer::isListening() const
{
    return listening;
}

void DtlsServer::close()
{
    listening = false;
}

void DtlsServer::readyRead()
{

    //! [3]
    const qint64 bytesToRead = serverSocket.pendingDatagramSize();
    if (bytesToRead <= 0) {
        emit warningMessage(tr("A spurious read notification"));
        return;
    }

    QByteArray dgram(bytesToRead, Qt::Uninitialized);
    QHostAddress peerAddress;
    quint16 peerPort = 0;
    const qint64 bytesRead = serverSocket.readDatagram(dgram.data(), dgram.size(),
                                         &peerAddress, &peerPort);
    if (bytesRead <= 0) {
        emit warningMessage(tr("Failed to read a datagram: ") + serverSocket.errorString());
        return;
    }
//    if (dgram.contains("00")){
//        int ret = 1;
//    }

    dgram.resize(bytesRead);
    //! [3]
    //! [4]
    if (peerAddress.isNull() || !peerPort) {
        emit warningMessage(tr("Failed to extract peer info (address, port)"));
        return;
    }

    const auto client = std::find_if(knownClients.begin(), knownClients.end(),
                                     [&](const std::unique_ptr<QDtls> &connection){
                                         return connection->peerAddress() == peerAddress
                                                && connection->peerPort() == peerPort;
                                     });
    //! [4]

    //! [5]
    if (client == knownClients.end())
        return handleNewConnection(peerAddress, peerPort, dgram);
    //! [5]

    //! [6]

    QDtls * dtlsServer = client->get();

    if (dtlsServer->isConnectionEncrypted()) {
        QSettings settings(SETTINGSFILE, QSettings::IniFormat);

        bool sendResponse = settings.value("sendReponse", false).toBool();
        bool sendSmartResponse = settings.value("sendSmartResponse", false).toBool();

        dgram = dtlsServer->decryptDatagram(&serverSocket, dgram);
        QByteArray closeNotify("close notify");



        std::vector<QString> recievedPacketInfo = createInfoVect(dtlsServer->peerAddress(), dtlsServer->peerPort(), serverSocket.localAddress(), serverSocket.localPort());
        if (dtlsServer->dtlsError() == QDtlsError::RemoteClosedConnectionError){
            dgram = closeNotify;
            closeNotifyReceived = true;
            //dtlsServer->shutdown(&serverSocket);
            sendResponse = false;
            knownClients.erase(client);

        }
        Packet recivedPacket = createPacket(recievedPacketInfo, dgram);
        emit serverPacketReceived(recivedPacket);

        if(settings.value("sendSimpleAck").toString() == "true" && !dgram.contains("close notify")){
        //if(settings.value("sendSimpleAck").toString() == "true" && closeNotifyReceived == false){
            sendAck(dtlsServer, dgram);
            return;
        } else {
            errorMessage("close notify recieved");
        }




        smartData.clear();
        //sendSmartResponse = true;
        if (sendSmartResponse) {
            QList<SmartResponseConfig> smartList;
            smartList.append(Packet::fetchSmartConfig(1, SETTINGSFILE));
            smartList.append(Packet::fetchSmartConfig(2, SETTINGSFILE));
            smartList.append(Packet::fetchSmartConfig(3, SETTINGSFILE));
            smartList.append(Packet::fetchSmartConfig(4, SETTINGSFILE));
            smartList.append(Packet::fetchSmartConfig(5, SETTINGSFILE));
            smartData = Packet::smartResponseMatch(smartList, dgram);
            if(!smartData.isEmpty()){
                //TODO: if(responseEnableCheck1 == true)
                if(serverResonse(client->get())){
                    return;
                }
            }


        }

        //sendResponse =false;

        if (sendResponse) {
            //if(!dgram.contains("close notify")){

                if(serverResonse(client->get())){
                    return;
    //                QMessageBox::critical(nullptr, "Connection Error", "server response can't be sent.");
                }
            //}

        }


//        if ((*client)->dtlsError() == QDtlsError::RemoteClosedConnectionError)
//            knownClients.erase(client);

        return;
    }
    //! [6]

    //! [7]
    doHandshake(client->get(), dgram);
    //! [7]
}

//! [13]
void DtlsServer::pskRequired(QSslPreSharedKeyAuthenticator *auth)
{
    Q_ASSERT(auth);

    emit infoMessage(tr("PSK callback, received a client's identity: ") + QString::fromLatin1(auth->identity()));
    auth->setPreSharedKey(QByteArrayLiteral("\x1a\x2b\x3c\x4d\x5e\x6f"));
}
//! [13]

//! [8]
void DtlsServer::handleNewConnection(const QHostAddress &peerAddress,
                                     quint16 peerPort, const QByteArray &clientHello)
{
    if (!listening)
        return;

    const QString peerInfo = peer_info(peerAddress, peerPort);
    if (cookieSender.verifyClient(&serverSocket, clientHello, peerAddress, peerPort)) {
        emit infoMessage(peerInfo + tr(": verified, starting a handshake"));
        //! [8]
        //! [9]
        std::unique_ptr<QDtls> newConnection{new QDtls{QSslSocket::SslServerMode}};
        newConnection->setDtlsConfiguration(serverConfiguration);
        newConnection->setPeer(peerAddress, peerPort);
        newConnection->connect(newConnection.get(), &QDtls::pskRequired,
                               this, &DtlsServer::pskRequired);
        knownClients.push_back(std::move(newConnection));
        doHandshake(knownClients.back().get(), clientHello);
        //! [9]
    } else if (cookieSender.dtlsError() != QDtlsError::NoError) {
        emit errorMessage(tr("DTLS error: ") + cookieSender.dtlsErrorString());
    } else {
        emit infoMessage(peerInfo + tr(": not verified yet"));
    }
}

//! [11]
void DtlsServer::doHandshake(QDtls *newConnection, const QByteArray &clientHello)
{
    const bool result = newConnection->doHandshake(&serverSocket, clientHello);
    if (!result) {
        emit errorMessage(newConnection->dtlsErrorString());
        return;
    }

    const QString peerInfo = peer_info(newConnection->peerAddress(),
                                       newConnection->peerPort());
    switch (newConnection->handshakeState()) {
    case QDtls::HandshakeInProgress:
        emit infoMessage(peerInfo + tr(": handshake is in progress ..."));
        break;
    case QDtls::HandshakeComplete:
        emit infoMessage(peerInfo + tr(": Connection encrypted. ") + connection_info(newConnection));
        break;
    default:
        Q_UNREACHABLE();
    }
}
//! [11]

//! [12]
void DtlsServer::sendAck(QDtls *connection, const QByteArray &clientMessage)
{
    Q_ASSERT(connection->isConnectionEncrypted());

    const QString peerInfo = peer_info(connection->peerAddress(), connection->peerPort());
    const QString serverInfo = peer_info(serverSocket.localAddress(), serverSocket.localPort());

    if (clientMessage.size()) {
        std::vector<QString> sentPacketInfo = createInfoVect(serverSocket.localAddress(), serverSocket.localPort(), connection->peerAddress(), connection->peerPort());
        Packet sentPacket = createPacket(sentPacketInfo, clientMessage);
        if(connection->isConnectionEncrypted()){

            if(connection->writeDatagramEncrypted(&serverSocket, QString("from %1: %2").arg(serverInfo, QString::fromUtf8(clientMessage)).toLatin1())){
                QString massageFromTheOtherPeer = "ACK: " + QString::fromUtf8(clientMessage);
                sentPacket.hexString = sentPacket.ASCIITohex(massageFromTheOtherPeer);
                emit serverPacketSent(sentPacket);
            }else{
                sentPacket.errorString = tr("Could not send response");
                emit serverPacketSent(sentPacket);
            }
        }
    } else if (connection->dtlsError() == QDtlsError::NoError) {
        emit warningMessage(peerInfo + ": " + tr("0 byte dgram, could be a re-connect attempt?"));
    } else {
        emit errorMessage(peerInfo + ": " + connection->dtlsErrorString());
    }
}
//! [12]

//! [14]
void DtlsServer::shutdown()
{
    for (const auto &connection : std::exchange(knownClients, {}))
        connection->shutdown(&serverSocket);

    serverSocket.close();
}
//! [14]

//this function using for creation packet that can be sent to packetReceivedECHO
Packet DtlsServer::createPacket(const std::vector<QString>& packetInfo, const QByteArray& dgram){

    Packet recPacket;
    recPacket.init();
    recPacket.fromIP = packetInfo[0];
    recPacket.fromPort = packetInfo[1].toUInt();
    recPacket.toIP = packetInfo[2];
    recPacket.port = packetInfo[3].toUInt();
    QString massageFromTheOtherPeer = QString::fromUtf8(dgram);
    recPacket.hexString = recPacket.ASCIITohex(massageFromTheOtherPeer);
    recPacket.tcpOrUdp = "DTLS";

    if(packetInfo[0] == "0.0.0.0"){
        recPacket.fromIP = "you";
    }
    if(packetInfo[2] == "0.0.0.0"){
        recPacket.toIP = "127.0.0.1";
    }

    return recPacket;


}

std::vector<QString> DtlsServer::createInfoVect(const QHostAddress &fromAddress, quint16 fromPort, const QHostAddress &toAddress, quint16 toPort){
    std::vector<QString> infoVect;
    infoVect.push_back(fromAddress.toString());
    infoVect.push_back(QString::number(fromPort));
    infoVect.push_back(toAddress.toString());
    infoVect.push_back(QString::number(toPort));
    return infoVect;

}

bool DtlsServer::serverResonse(QDtls* dtlsServer){

    Packet responsePacket;
    responsePacket.init();

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString responseData = (settings.value("responseHex", "")).toString();

    responsePacket.timestamp = QDateTime::currentDateTime();
    responsePacket.name = responsePacket.timestamp.toString(DATETIMEFORMAT);
    responsePacket.tcpOrUdp = "DTLS";
    responsePacket.fromIP = "You";
    bool isIPv6  = IPv6Enabled();
    if (isIPv6) {
        responsePacket.toIP = Packet::removeIPv6Mapping(dtlsServer->peerAddress());

    } else {
        responsePacket.toIP = (dtlsServer->peerAddress()).toString();
    }
    responsePacket.port = dtlsServer->peerPort();
    responsePacket.fromPort = serverSocket.localPort();
    responsePacket.hexString = responseData;
    QString testMacro = Packet::macroSwap(responsePacket.asciiString());
    responsePacket.hexString = Packet::ASCIITohex(testMacro);

    if (!smartData.isEmpty()) {
        responsePacket.hexString = Packet::byteArrayToHex(smartData);
    }

    serverSocket.waitForBytesWritten();

    if(dtlsServer->isConnectionEncrypted()){
        if(dtlsServer->writeDatagramEncrypted(&serverSocket,responsePacket.getByteArray())){
            emit serverPacketSent(responsePacket);
            return true;
        }else{
            responsePacket.errorString = "Could not send response";
            emit serverPacketSent(responsePacket);
            return false;

        }
    }

    return false;

}



bool DtlsServer::IPv6Enabled()
{
    return !IPv4Enabled();
}

bool DtlsServer::IPv4Enabled()
{
    QString ipMode = getIPmode();
    if(ipMode == "4") {
        return true;
    }
    return (ipMode.contains("v4") || ipMode.contains("."));
}

QString DtlsServer::getIPmode()
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

QHostAddress DtlsServer::resolveDNS(QString hostname)
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

void DtlsServer::loadKeyLocalCertCaCert(){
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QString localCertPath = settings.value("sslLocalCertificatePath", CERTFILE).toString();
    QFile certFile(localCertPath);

    if(!certFile.open(QIODevice::ReadOnly) && !(localCertPath.isEmpty())){
        QMessageBox::critical(nullptr, "Certificate Error", "Local certificate path can't opened.");
        return;
    }

    //getCertFormat
    QSsl::EncodingFormat format = getCertFormat(certFile);
    QSslCertificate currentCertificate(&certFile, format);
    certificate = currentCertificate;
    if (currentCertificate.isNull() && !(localCertPath.isEmpty())) {
        QMessageBox::critical(nullptr, "Certificate Error", "Local certificate is not valid.");
        return;
    }

    QString keyPath = settings.value("sslPrivateKeyPath", KEYFILE).toString();
    QFile keyFile(keyPath);

    if(!keyFile.open(QIODevice::ReadOnly) && !(keyPath.isEmpty())){
        QMessageBox::critical(nullptr, "Key Error", "Key path can't opened.");
        return;
    }

    privateKey = getPrivateKey(keyFile);



    //get the full path to to ca-signed-cert.pem file
    QString fullCaCertPath = getFullPathToCaCert();
    QFile caCertFile(fullCaCertPath);

    if(!caCertFile.open(QIODevice::ReadOnly) && !(fullCaCertPath.isEmpty())){
        QMessageBox::critical(nullptr, "Ca-Certificate Error", "Ca-Certificate path can't opened.");
        return;
    }

    QSsl::EncodingFormat formatCa = getCertFormat(caCertFile);
    QSslCertificate currentCaCertificate(&caCertFile, formatCa);
    caCertificate = currentCaCertificate;

    if (currentCaCertificate.isNull() && !(fullCaCertPath.isEmpty())) {
        QMessageBox::critical(nullptr, "Ca-Certificate Error", "Ca-Certificate is not valid.");
        return;
    }

    setConfiguration();
}

void DtlsServer::setConfiguration(){
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    serverConfiguration = QSslConfiguration::defaultDtlsConfiguration();
    serverConfiguration.setLocalCertificate(certificate);
    serverConfiguration.setPrivateKey(privateKey);
    serverConfiguration.setCaCertificates(QList<QSslCertificate>() << caCertificate);
    if(settings.value("twoVerify").toString() == "true"){
        serverConfiguration.setPeerVerifyMode(QSslSocket::VerifyPeer);
    } else{
        serverConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    }
}

void DtlsServer::on_signedCert_textChanged(){
    loadKeyLocalCertCaCert();
}

QSsl::EncodingFormat DtlsServer::getCertFormat(QFile& certFile){
    QFileInfo fileInfo(certFile.fileName());
    QString fileExtension = fileInfo.suffix().toLower();
    QSsl::EncodingFormat format = QSsl::Pem;

    if (fileExtension == "pem") {
        format = QSsl::Pem;
    } else if (fileExtension == "der") {
        format = QSsl::Der;
    }
    return format;
}

QSslKey DtlsServer::getPrivateKey(QFile& keyFile){
    QList<QSsl::KeyAlgorithm> keyTypes = { QSsl::Dh, QSsl::Dsa, QSsl::Ec, QSsl::Rsa };
    QSslKey privateKey;
    foreach (QSsl::KeyAlgorithm type, keyTypes) {
        QSslKey key(&keyFile, type);
        if (!key.isNull()) {
            privateKey = key;
            break;
        }
        keyFile.reset();
    }
    if(privateKey.isNull() && keyFile.isOpen()){
        QMessageBox::critical(nullptr, "Key Error", "Key is not valid.");
        return QSslKey();
    }
    return privateKey;
}

QString DtlsServer::getFullPathToCaCert(){
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString caCertFolder = settings.value("sslCaPath", KEYFILE).toString();
    QString fullCaCertPath;
    QDir dir(caCertFolder);
    if (dir.exists()) {
        QStringList nameFilters;
        nameFilters << "*.pem" << "*.der";  // Filter for .txt files

        dir.setNameFilters(nameFilters);
        QStringList fileList = dir.entryList();

        if (!fileList.isEmpty()) {
            // Select the first file that matches the filter
            fullCaCertPath = dir.filePath(fileList.first());
        } else {
            qDebug() << "No matching files found.";
        }
    } else {
        qDebug() << "Directory does not exist.";
    }
    return fullCaCertPath;
}
#else
void DtlsServer::on_signedCert_textChanged(){

}


#endif
