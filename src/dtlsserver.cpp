// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dtlsserver.h"

#include <algorithm>

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
    /////////////////////////////////////////////////////
    QFile certFile("C:/Users/israe/OneDrive - ort braude college of engineering/rsa_encryption/server-signed-cert.pem");
    if(!certFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslCertificate certificate(&certFile, QSsl::Pem);

    QFile keyFile("C:/Users/israe/OneDrive - ort braude college of engineering/rsa_encryption/server-key.pem");
    if(!keyFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslKey privateKey(&keyFile, QSsl::Rsa); // Or QSsl::Ec if your key is ECDSA

    QFile caCertFile("C:/Users/israe/OneDrive - ort braude college of engineering/rsa_encryption/ca-signed-cert/signed-cert.pem");
    if(!caCertFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslCertificate caCertificate(&caCertFile, QSsl::Pem);

    serverConfiguration = QSslConfiguration::defaultDtlsConfiguration();
    serverConfiguration.setLocalCertificate(certificate);
    serverConfiguration.setPrivateKey(privateKey);
    serverConfiguration.setCaCertificates(QList<QSslCertificate>() << caCertificate);
    serverConfiguration.setPeerVerifyMode(QSslSocket::VerifyPeer);
    ///////////////////////////////////////////////////////////////////

    //serverConfiguration = QSslConfiguration::defaultDtlsConfiguration();
    //serverConfiguration.setPreSharedKeyIdentityHint("Qt DTLS example server");
    //serverConfiguration.setPeerVerifyMode(QSslSocket::VerifyPeer);
    //cookieSender.setDtlsConfiguration(dtlsConfiguration);
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
            emit errorMessage(serverSocket.errorString());
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
    if ((*client)->isConnectionEncrypted()) {
        decryptDatagram(client->get(), dgram);
        if ((*client)->dtlsError() == QDtlsError::RemoteClosedConnectionError)
            knownClients.erase(client);
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

    emit infoMessage(tr("PSK callback, received a client's identity: '%1'")
                         .arg(QString::fromLatin1(auth->identity())));
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
        emit infoMessage(tr("Connection with %1 encrypted. %2")
                             .arg(peerInfo, connection_info(newConnection)));
        break;
    default:
        Q_UNREACHABLE();
    }
}
//! [11]

//! [12]
void DtlsServer::decryptDatagram(QDtls *connection, const QByteArray &clientMessage)
{
    Q_ASSERT(connection->isConnectionEncrypted());

    const QString peerInfo = peer_info(connection->peerAddress(), connection->peerPort());
    const QByteArray dgram = connection->decryptDatagram(&serverSocket, clientMessage);

    if (dgram.size()) {
        //the vector content: createInfoVect(const QHostAddress &fromAddress, quint16 fromPort, const QHostAddress &toAddress, quint16 toPort)
        //TODO: insert the vector error massage
        std::vector<QString> recievedPacketInfo = createInfoVect(connection->peerAddress(), connection->peerPort(), serverSocket.localAddress(), serverSocket.localPort());
        Packet recivedPacket = createPacket(recievedPacketInfo, clientMessage, dgram);
        emit serverPacketReceived(recivedPacket);

        if(connection->writeDatagramEncrypted(&serverSocket, tr("to %1: ACK").arg(peerInfo).toLatin1())){
            std::vector<QString> sentPacketInfo = createInfoVect(serverSocket.localAddress(), serverSocket.localPort(), connection->peerAddress(), connection->peerPort());
            Packet sentPacket = createPacket(sentPacketInfo, clientMessage, dgram);
            emit serverPacketSent(sentPacket);
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
//    void datagramReceived(const QString &peerInfo, const QByteArray &cipherText, const QByteArray &plainText);
//    void receivedDatagram(QString & peerInfo, QByteArray &clientMessage, QByteArray dgram);

Packet DtlsServer::createPacket(const std::vector<QString>& packetInfo, const QByteArray &clientMessage, const QByteArray& dgram){

    Packet recPacket;
    recPacket.init();
    recPacket.fromIP = packetInfo[0];
    recPacket.fromPort = packetInfo[1].toUInt();
    recPacket.toIP = packetInfo[2];
    recPacket.port = packetInfo[3].toUInt();
    QString massageFromTheOtherPeer = QString::fromUtf8(dgram);
    recPacket.hexString = massageFromTheOtherPeer;
    recPacket.errorString = "none";
    recPacket.tcpOrUdp = "DTLS";

    if((packetInfo[0] == "0.0.0.0") || (packetInfo[0] == "127.0.0.1")){
        recPacket.fromIP = "You";
    }
    if((packetInfo[2] == "0.0.0.0") || (packetInfo[2] == "127.0.0.1")){
        recPacket.toIP = "You";
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

//void DtlsServer::serverSentDatagram(const QString& peerInfo, const QByteArray &clientMessage, const QByteArray& dgram){
//    Packet recPacket;
//    recPacket.init();
//    recPacket.fromIP = "You";
//    QStringList info = peerInfo.split(":");
//    recPacket.toIP = info[0].remove(0, 1);
//    recPacket.fromPort = info[1].remove(info[1].length() - 1, 1).toUInt();
//    recPacket.port = serverSocket.localPort();
//    QString massageFromTheOtherPeer = QString::fromUtf8(dgram);
//    recPacket.hexString = massageFromTheOtherPeer;
//    recPacket.errorString = "none";
//    recPacket.tcpOrUdp = "DTLS";

//    emit serverPacketSent(recPacket);

//}

//std::vector<QString> getPacketInfo(const QHostAddress &fromAddress, quint16 fromPort, const QHostAddress &toAddress, quint16 toPort){
//    std::vector<QString> infoVect;
//    infoVect.push_back();
//}
