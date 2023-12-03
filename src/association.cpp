// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "association.h"
#include "packet.h"

DtlsAssociation::DtlsAssociation(const QHostAddress &address, quint16 port,
                                 const QString &connectionName, std::vector<QString> cmdComponents)
    : name(connectionName),
    crypto(QSslSocket::SslClientMode)
{

    QFile certFile(cmdComponents[4]);//4
    if(!certFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslCertificate certificate(&certFile, QSsl::Pem);

    QFile keyFile(cmdComponents[3]);//3
    if(!keyFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslKey privateKey(&keyFile, QSsl::Rsa); // Or QSsl::Ec if your key is ECDSA

    QFile caCertFile(cmdComponents[5]);//5
    if(!caCertFile.open(QIODevice::ReadOnly)){
        return;
    }
    QSslCertificate caCertificate(&caCertFile, QSsl::Pem);

    configuration.setLocalCertificate(certificate);
    configuration.setPrivateKey(privateKey);
    configuration.setCaCertificates(QList<QSslCertificate>() << caCertificate);

    configuration.setPeerVerifyMode(QSslSocket::VerifyNone);
    crypto.setPeer(address, port);
    crypto.setDtlsConfiguration(configuration);
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
    if (socket.state() != QAbstractSocket::ConnectedState) {
        emit infoMessage(tr("%1: connecting UDP socket first ...").arg(name));
        connect(&socket, &QAbstractSocket::connected, this, &DtlsAssociation::udpSocketConnected);
        return;
    }

    if (!crypto.doHandshake(&socket))
        emit errorMessage(tr("%1: failed to start a handshake - %2").arg(name, crypto.dtlsErrorString()));
    else{
        while(true){
            socket.waitForReadyRead();
            if(crypto.isConnectionEncrypted()){
                break;
            }
        }
        emit infoMessage(tr("%1: starting a handshake").arg(name));
    }

}
//! [5]

void DtlsAssociation::udpSocketConnected()
{
    emit infoMessage(tr("%1: UDP socket is now in ConnectedState, continue with handshake ...").arg(name));
    startHandshake();
}


void DtlsAssociation::readyRead()
{

    if (socket.pendingDatagramSize() <= 0) {
        emit warningMessage(tr("%1: spurious read notification?").arg(name));
        return;
    }

    //! [6]
    QByteArray dgram(socket.pendingDatagramSize(), Qt::Uninitialized);
    const qint64 bytesRead = socket.readDatagram(dgram.data(), dgram.size());
    if (bytesRead <= 0) {
        emit warningMessage(tr("%1: spurious read notification?").arg(name));
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
            emit errorMessage(tr("%1: shutdown alert received").arg(name));
            socket.close();
            pingTimer.stop();
            return;
        }

        emit warningMessage(tr("%1: zero-length datagram received?").arg(name));
    } else {
        //! [7]
        //! [8]
        if (!crypto.doHandshake(&socket, dgram)) {
            emit errorMessage(tr("%1: handshake error - %2").arg(name, crypto.dtlsErrorString()));
            return;
        }
        //! [8]
        crypto.doHandshake(&socket, dgram);
        //! [9]
        if (crypto.isConnectionEncrypted()) {
            emit infoMessage(tr("%1: encrypted connection established!").arg(name));
            emit handShakeComplited();
        } else {
            //! [9]
            emit infoMessage(tr("%1: continuing with handshake ...").arg(name));
        }

    }


}


//! [11]
void DtlsAssociation::handshakeTimeout()
{
    emit warningMessage(tr("%1: handshake timeout, trying to re-transmit").arg(name));
    if (!crypto.handleTimeout(&socket))
        emit errorMessage(tr("%1: failed to re-transmit - %2").arg(name, crypto.dtlsErrorString()));
}
//! [11]

//! [14]
void DtlsAssociation::pskRequired(QSslPreSharedKeyAuthenticator *auth)
{
    Q_ASSERT(auth);

    emit infoMessage(tr("%1: providing pre-shared key ...").arg(name));
    auth->setIdentity(name.toLatin1());
    auth->setPreSharedKey(QByteArrayLiteral("\x1a\x2b\x3c\x4d\x5e\x6f"));
}


void DtlsAssociation::setCipher(QString chosenCipher) {
    configuration.setCiphers(chosenCipher);
    //configuration.setProtocol(QSsl::DtlsV1_2);
    crypto.setDtlsConfiguration(configuration);
}

///////////////////////////////////////////////////backups//////////////////////////////////
//! [14]

//! [10]
//!
//!
//! only for ping massage
//void DtlsAssociation::pingTimeout()
//{

//    //static const QString message = QStringLiteral("I am %1, please, accept our ping %2");
//    //const qint64 written = crypto.writeDatagramEncrypted(&socket, message.arg(name).arg(ping).toLatin1());
//    if(this->newMassageToSend){
//        emit handShakeComplited();
//        const qint64 written = crypto.writeDatagramEncrypted(&socket, massageToSend.toLatin1());
//        if (written <= 0) {
//            emit errorMessage(tr("%1: failed to send a ping - %2").arg(name, crypto.dtlsErrorString()));
//            pingTimer.stop();
//            return;
//        }
//        this->newMassageToSend = false;
//        ++ping;
//    }
//}
//! [10]




//void DtlsAssociation::setKeyCertAndCaCert(QString keyPath, QString certPath, QString caPath) {
//    QFile keyFile(keyPath);
//    QFile certFile(certPath);
//    QFile caFile(caPath);
//    if (certFile.open(QIODevice::ReadOnly) && keyFile.open(QIODevice::ReadOnly) && caFile.open(QIODevice::ReadOnly)) {
//        QSslKey privateKey(&keyFile, QSsl::Rsa, QSsl::Pem);
//        QSslCertificate certificate(&certFile, QSsl::Pem);
//        QSslCertificate caCertificate(&caFile, QSsl::Pem);

//        configuration.setPrivateKey(privateKey);
//        configuration.setLocalCertificate(certificate);
//        configuration.setCaCertificates(QList<QSslCertificate>() << caCertificate);

//        crypto.setDtlsConfiguration(configuration);
//    }
//    else {
//        //QDebug("Error loading certs or key");
//    }
//}
////////////////////////
//QFile certFile("C:/Users/israe/OneDrive - ort braude college of engineering/rsa_encryption/client-signed-cert.pem");
//if(!certFile.open(QIODevice::ReadOnly)){
//    return;
//}
//QSslCertificate certificate(&certFile, QSsl::Pem);

//QFile keyFile("C:/Users/israe/OneDrive - ort braude college of engineering/rsa_encryption/client-key.pem");
//if(!keyFile.open(QIODevice::ReadOnly)){
//    return;
//}
//QSslKey privateKey(&keyFile, QSsl::Rsa); // Or QSsl::Ec if your key is ECDSA

//QFile caCertFile("C:/Users/israe/OneDrive - ort braude college of engineering/rsa_encryption/ca-signed-cert/signed-cert.pem");
//if(!caCertFile.open(QIODevice::ReadOnly)){
//    return;
//}
//QSslCertificate caCertificate(&caCertFile, QSsl::Pem);

////auto configuration = QSslConfiguration::defaultDtlsConfiguration();
//configuration.setCiphers("AES256-GCM-SHA384");

//configuration.setLocalCertificate(certificate);
//configuration.setPrivateKey(privateKey);
//configuration.setCaCertificates(QList<QSslCertificate>() << caCertificate);
////////////////////////
