/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#ifndef PACKETNETWORK_H
#define PACKETNETWORK_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QStringList>
#include <QTime>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QHash>
#include "globals.h"
#include "tcpthread.h"
#include "packet.h"
#include "persistentconnection.h"


class PacketNetwork : public QTcpServer
{
    Q_OBJECT
public:
    explicit PacketNetwork( QWidget *parent = 0);
    void init();

    QString debugQByteArray(QByteArray debugArray);
    int getUDPPort();
    int getTCPPort();

    void kill();
    QString responseData;
    bool sendResponse;
    bool smartResponseEnableCheck;
    bool activateUDP;
    bool activateTCP;
    bool receiveBeforeSend;
    int delayAfterConnect;
    bool persistentConnectCheck;
    void setIPmode(int mode);
    static int getIPmode();

    QList<SmartResponseConfig> smartList;


protected:
    void incomingConnection(qintptr socketDescriptor);

signals:
    void packetReceived(Packet sendpacket);
    void toStatusBar(const QString & message, int timeout = 0, bool override = false);
    void packetSent(Packet sendpacket);


public slots:
    void packetReceivedECHO(Packet sendpacket);
    void toStatusBarECHO(const QString & message, int timeout = 0, bool override = false);
    void packetSentECHO(Packet sendpacket);


public slots:
    void readPendingDatagrams();
    void disconnected();
    void packetToSend(Packet sendpacket);

private slots:
     void newSession();

private:

    QUdpSocket *udpSocket;
    QList<TCPThread *> tcpthreadList;
    QList<PersistentConnection *> pcList;

    SmartResponseConfig loadSmartConfig(int num);
};

#endif // PACKETNETWORK_H
