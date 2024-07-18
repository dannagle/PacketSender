// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "association.h"
#include "packet.h"

DtlsAssociation::DtlsAssociation(QHostAddress &address, quint16 port,
                                 const QString &connectionName, std::vector<QString> cmdComponents)
    : name(connectionName),
    crypto(QSslSocket::SslClientMode)
{


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
        QHostInfo host = QHostInfo::fromName(cmdComponents[1]);
        // Check if the lookup was successful
        if (host.error() != QHostInfo::NoError) {
            packetToSend.errorString += "Lookup failed:" + host.errorString();
            qDebug() << "Lookup failed:" << host.errorString();
        } else {
            // Output the host name
            foreach (const QHostAddress &resolvedAddress, host.addresses()) {
                //if it is an ipv4 save it as addres and fill the hostName with the current hostName
                if (resolvedAddress.protocol() == QAbstractSocket::IPv4Protocol){
                    address = resolvedAddress;
                    hostName = cmdComponents[1];
                }

            }
        }
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
    if (socket.state() != QAbstractSocket::ConnectedState) {
        emit infoMessage(tr("%1: connecting UDP socket first ...").arg(name));
        connect(&socket, &QAbstractSocket::connected, this, &DtlsAssociation::udpSocketConnected);
        return;
    }

    if (!crypto.doHandshake(&socket)){
        packetToSend.errorString += " Failed to start a handshake ";
        emit errorMessage(tr("%1: failed to start a handshake - %2").arg(name, crypto.dtlsErrorString()));
    }
    else{
        while(true){
            socket.waitForReadyRead();
            if(crypto.isConnectionEncrypted() || closeRequest){

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
//            packetToSend.errorString += " Shutdown alert received";
//            emit errorMessage(tr("%1: shutdown alert received").arg(name));
            //socket.close();
            pingTimer.stop();
            return;
        }

        emit warningMessage(tr("%1: zero-length datagram received?").arg(name));
    } else {
        //! [7]
        //! [8]
        if (!crypto.doHandshake(&socket, dgram)) {
            packetToSend.errorString += " handshake error ";
            emit errorMessage(tr("%1: handshake error - %2").arg(name, crypto.dtlsErrorString()));
            return;
        }
        //! [8]
        //crypto.doHandshake(&socket, dgram);
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
        packetToSend.errorString += " Failed to re-transmit ";

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

#if QT_VERSION > QT_VERSION_CHECK(6, 00, 0)
    configuration.setCiphers(chosenCipher);
    crypto.setDtlsConfiguration(configuration);
#else
    //TODO support this with Qt5
    QDEBUG() << "This is disabled for Qt5 (for now)";
#endif

}

QSsl::EncodingFormat DtlsAssociation::getCertFormat(QFile& certFile){
    QFileInfo fileInfo(certFile.fileName());
    QString fileExtension = fileInfo.suffix().toLower();
    QSsl::EncodingFormat format;

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




