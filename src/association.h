// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include <QtNetwork>
#include <QtCore>
#include "packet.h"

//! [0]
class DtlsAssociation : public QObject
{
    Q_OBJECT

public:
    QDtls crypto;
    bool newMassageToSend = false;
    QString massageToSend;
    QUdpSocket socket;
    QString name;
    Packet packetToSend;

    DtlsAssociation(const QHostAddress &address, quint16 port,
                    const QString &connectionName, Packet packetToSend);
    ~DtlsAssociation();
    void startHandshake();
    void setKeyCertAndCaCert(QString keyPath, QString certPath, QString caPath);
    void setCipher(QString chosenCipher);
    QSslConfiguration configuration = QSslConfiguration::defaultDtlsConfiguration();

signals:
    void handShakeComplited(Packet packetToSend, DtlsAssociation* dtlsAssociation);
    void errorMessage(const QString &message);
    void warningMessage(const QString &message);
    void infoMessage(const QString &message);
    void serverResponse(const QString &clientInfo, const QByteArray &datagraam,
                        const QByteArray &plainText, QHostAddress peerAddress, quint16 peerPort, quint16 clientPort);

private slots:
    void udpSocketConnected();
    void readyRead();
    void handshakeTimeout();
    void pskRequired(QSslPreSharedKeyAuthenticator *auth);
    void pingTimeout();
    //void writeMassage();


private:


    QTimer pingTimer;
    unsigned ping = 0;

    Q_DISABLE_COPY(DtlsAssociation)
};
//! [0]

#endif // ASSOCIATION_H
