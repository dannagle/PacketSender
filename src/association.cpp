// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "association.h"
#include "packet.h"
#include "globals.h"
#include <QTimer>
#include <QCoreApplication>
#include <QDtls>

#if QT_VERSION > QT_VERSION_CHECK(6, 00, 0)

DtlsAssociation::DtlsAssociation(QHostAddress &address, quint16 port,
                                 const QString &connectionName, std::vector<QString> cmdComponents)
    : crypto(QSslSocket::SslClientMode),
    name(connectionName)
{

    QDEBUG();

    QFile certFile(cmdComponents[4]);//4
    if(!certFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSsl::EncodingFormat format = getCertFormat(certFile);
    QSslCertificate certificate(&certFile, format);
    if (certificate.isNull()) {
        packetToSend.errorString += "Your local certificate content isn't structured as any certificate format";
    }

    //key
    QFile keyFile(cmdComponents[3]);//3
    if(!keyFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslKey privateKey = getPrivateKey(keyFile);

    //ca-cert
    QFile caCertFile(cmdComponents[5]);//5
    if(!caCertFile.open(QIODevice::ReadOnly)){
        return;
    }
    //getCertFormat
    QSsl::EncodingFormat formatCa = getCertFormat(caCertFile);
    QSslCertificate caCertificate(&caCertFile, formatCa);
    if (caCertificate.isNull()) {
        packetToSend.errorString += "Your ca-certificate content isn't structured as any certificate format";
    }
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString hostName = settings.value("hostNameEdit").toString();

    //check if the address field contains a valid host name instead of implicite address
    if (hostName.isEmpty()){
        hostName = "empty host name";
    }

    configuration.setLocalCertificate(certificate);
    configuration.setPrivateKey(privateKey);
    configuration.setCaCertificates(QList<QSslCertificate>() << caCertificate);

    configuration.setPeerVerifyMode(QSslSocket::VerifyPeer);
    crypto.setPeer(address, port);
    crypto.setPeerVerificationName(hostName);
    crypto.setDtlsConfiguration(configuration);

    hostName = "";
    connect(&crypto, &QDtls::pskRequired, this, &DtlsAssociation::pskRequired);
    //! [3]
    socket.connectToHost(address.toString(), port);
    socket.waitForConnected();
    //! [3]
    //! [13]
    connect(&socket, &QUdpSocket::readyRead, this, &DtlsAssociation::readyRead);
    //! [13]

}

//! [12]
DtlsAssociation::~DtlsAssociation()
{
    if (crypto.isConnectionEncrypted())
        crypto.shutdown(&socket);
}
//! [12]

//! [5]
void DtlsAssociation::startHandshake()
{
    QDEBUG();

    Packet errorPacket;
    errorPacket.init();
    errorPacket.timestamp = QDateTime::currentDateTime();
    errorPacket.tcpOrUdp = "DTLS";
    errorPacket.name = errorPacket.timestamp.toString(DATETIMEFORMAT);
    errorPacket.toIP = "You";
    errorPacket.port = socket.localPort();
    errorPacket.fromPort = socket.peerPort();
    errorPacket.fromIP = socket.peerAddress().toString();

    QDEBUG();
    if (socket.state() != QAbstractSocket::ConnectedState) {

        QDEBUG();
        errorPacket.errorString = "connecting UDP socket first";
        QDEBUG();
        emit packetSent(errorPacket);
        QDEBUG() << connect(&socket, &QAbstractSocket::connected, this, &DtlsAssociation::udpSocketConnected);
        QDEBUG();
        return;
    }

    QDEBUG();
    if (!crypto.doHandshake(&socket)){
        QDEBUG();
        //socket.waitForBytesWritten();
        errorPacket.errorString = "Failed to start a handshake " + crypto.dtlsErrorString();
        emit packetSent(errorPacket);

    }
    else{
        QDEBUG();

        while(true){
            socket.waitForReadyRead(2000);
            if(crypto.isConnectionEncrypted() || closeRequest){


                break;

            }
        }
        QDEBUG() << (name + tr(": starting a handshake"));
    }

    QDEBUG();


    QDEBUG();
    QList<QSslError> sslErrorsList  = crypto.peerVerificationErrors();


    QDEBUGVAR(sslErrorsList.size());

    if (sslErrorsList.size() > 0) {

        QSslError sError;
        foreach (sError, sslErrorsList) {
            errorPacket.hexString.clear();
            errorPacket.errorString = sError.errorString();
            emit packetSent(errorPacket);
        }

    }

    if (crypto.isConnectionEncrypted()) {
        QDEBUG();
        QSslCipher cipher = crypto.sessionCipher();
        errorPacket.hexString.clear();
        errorPacket.errorString = "Encrypted with " + cipher.encryptionMethod();
        QDEBUGVAR(cipher.encryptionMethod());
        emit packetSent(errorPacket);

        errorPacket.hexString.clear();
        errorPacket.errorString = "Authenticated with " + cipher.authenticationMethod();
        QDEBUGVAR(cipher.encryptionMethod());
        emit packetSent(errorPacket);

    } else {


    }

    QDEBUG();

}
//! [5]

void DtlsAssociation::udpSocketConnected()
{
    QDEBUG();

    Packet errorPacket;
    errorPacket.init();
    errorPacket.timestamp = QDateTime::currentDateTime();
    errorPacket.tcpOrUdp = "DTLS";
    errorPacket.name = errorPacket.timestamp.toString(DATETIMEFORMAT);
    errorPacket.toIP = "You";
    errorPacket.port = socket.localPort();
    errorPacket.fromPort = socket.peerPort();
    errorPacket.fromIP = socket.peerAddress().toString();

    errorPacket.errorString = " UDP socket is now in ConnectedState, continue with handshake";
    emit packetSent(errorPacket);
    startHandshake();
}


void DtlsAssociation::readyRead()
{
    QDEBUG();

    Packet errorPacket;
    errorPacket.init();
    errorPacket.timestamp = QDateTime::currentDateTime();
    errorPacket.tcpOrUdp = "DTLS";
    errorPacket.name = errorPacket.timestamp.toString(DATETIMEFORMAT);
    errorPacket.toIP = "You";
    errorPacket.port = socket.localPort();
    errorPacket.fromPort = socket.peerPort();
    errorPacket.fromIP = socket.peerAddress().toString();


    if (socket.pendingDatagramSize() <= 0) {
        QDEBUG() << (name + tr(": spurious read notification?"));
        return;
    }

    //! [6]
    QByteArray dgram(socket.pendingDatagramSize(), Qt::Uninitialized);
    const qint64 bytesRead = socket.readDatagram(dgram.data(), dgram.size());
    if (bytesRead <= 0) {
        QDEBUG() << (name + tr(": spurious read notification?"));
        return;
    }

    dgram.resize(bytesRead);
    //! [6]
    //! [7]
    if (crypto.isConnectionEncrypted()) {
        const QByteArray plainText = crypto.decryptDatagram(&socket, dgram);
        if (plainText.size()) {
            emit receivedDatagram(plainText);
            return;
        }

        if (crypto.dtlsError() == QDtlsError::RemoteClosedConnectionError) {
//            errorPacket.errorString += " Shutdown alert received";
//            QDEBUG() << (name + tr("%1: shutdown alert received"));
            //socket.close();
            pingTimer.stop();
            return;
        }

        QDEBUG() << (name + tr(": zero-length datagram received?"));
    } else {
        //! [7]
        //! [8]
        QThread::msleep(HANDSHAKE_STEPS_TIMEOUT);
        if (!crypto.doHandshake(&socket, dgram)) {

            auto sslErrorsList = crypto.peerVerificationErrors();
            crypto.ignoreVerificationErrors(crypto.peerVerificationErrors());

            if (sslErrorsList.size() > 0) {

                QSslError sError;
                foreach (sError, sslErrorsList) {
                    errorPacket.hexString.clear();
                    errorPacket.errorString = sError.errorString();
                    emit packetSent(errorPacket);
                }

            }

            crypto.resumeHandshake(&socket);
            emit packetSent(errorPacket);
            return;
        }
        //! [8]
        //crypto.doHandshake(&socket, dgram);
        //! [9]
        if (crypto.isConnectionEncrypted()) {
            errorPacket.errorString = "encrypted connection established";
            emit packetSent(errorPacket);
            emit handShakeComplited();
        } else {
            //! [9]
            QDEBUG() << (name + tr(": continuing with handshake ..."));
        }

    }


}


//! [11]
void DtlsAssociation::handshakeTimeout()
{
    QDEBUG();

    QDEBUG() << (name + tr(": handshake timeout, trying to re-transmit"));
    if (!crypto.handleTimeout(&socket))
        packetToSend.errorString += " Failed to re-transmit ";

        QDEBUG() << (name + tr(": failed to re-transmit - ") + crypto.dtlsErrorString());
}
//! [11]

//! [14]
void DtlsAssociation::pskRequired(QSslPreSharedKeyAuthenticator *auth)
{
    Q_ASSERT(auth);

    QDEBUG() << (name + tr(": providing pre-shared key ..."));
    auth->setIdentity(name.toLatin1());
    auth->setPreSharedKey(QByteArrayLiteral("\x1a\x2b\x3c\x4d\x5e\x6f"));
}


void DtlsAssociation::setCipher(QString chosenCipher) {

    QDEBUG();
    configuration.setCiphers(chosenCipher);
    crypto.setDtlsConfiguration(configuration);
}

QSsl::EncodingFormat DtlsAssociation::getCertFormat(QFile& certFile){
    QFileInfo fileInfo(certFile.fileName());
    QString fileExtension = fileInfo.suffix().toLower();
    QSsl::EncodingFormat format = QSsl::Pem;

    QDEBUG();
    if (fileExtension == "pem") {
        format = QSsl::Pem;
    } else if (fileExtension == "der") {
        format = QSsl::Der;
    }
    return format;
}

QSslKey DtlsAssociation::getPrivateKey(QFile& keyFile){
    QList<QSsl::KeyAlgorithm> keyTypes = { QSsl::Dh, QSsl::Dsa, QSsl::Ec, QSsl::Rsa };
    QSslKey privateKey;

    QDEBUG();
    foreach (QSsl::KeyAlgorithm type, keyTypes) {
        QSslKey key(&keyFile, type);
        if (!key.isNull()) {
            privateKey = key;
            break;
        }
        keyFile.reset();
    }
    if(privateKey.isNull()){
        packetToSend.errorString += "Your key isn't one of the known key's types: Dh, Dsa, Ec, Rsa";
    }
    return privateKey;
}

#else

#endif



