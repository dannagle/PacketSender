// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#pragma once

#include <QtNetwork>
#include <QtCore>
#include "packet.h"


#if QT_VERSION > QT_VERSION_CHECK(6, 00, 0)


//! [0]
class DtlsAssociation : public QObject
{
    Q_OBJECT

public:
    DtlsAssociation(QHostAddress &address, quint16 port,
                    const QString &connectionName, std::vector<QString> cmdComponents);
    ~DtlsAssociation();
    void startHandshake();
    void setCipher(QString chosenCipher);
    QSsl::EncodingFormat getCertFormat(QFile& certFile);
    QSslKey getPrivateKey(QFile& keyFile);


    QSslConfiguration configuration = QSslConfiguration::defaultDtlsConfiguration();
    QDtls crypto;
    QUdpSocket socket;
    QString name;
    Packet packetToSend;

    bool closeRequest;

signals:
    void errorMessage(const QString &message);
    void warningMessage(const QString &message);
    void infoMessage(const QString &message);
    void handShakeComplited();
    void receivedDatagram(QByteArray plainText);
    void packetSent(Packet);

private slots:
    void udpSocketConnected();
    void readyRead();
    void pskRequired(QSslPreSharedKeyAuthenticator *auth);
    void handshakeTimeout();

private:


    QTimer pingTimer;
    unsigned ping = 0;

    Q_DISABLE_COPY(DtlsAssociation)
};

#else
class DtlsAssociation : public QThread
{
    Q_OBJECT

public:
    int nothing;

};


#endif
