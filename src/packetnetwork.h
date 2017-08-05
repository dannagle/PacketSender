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
#include <threadedtcpserver.h>


class PacketNetwork : public QObject
{
    Q_OBJECT
public:
    explicit PacketNetwork( QWidget *parent = 0);
    void init();

    QString debugQByteArray(QByteArray debugArray);
    int getUDPPort();
    int getTCPPort();
    int getSSLPort();

    void kill();
    QString responseData;
    bool sendResponse;
    bool sendSmartResponse;
    bool activateUDP;
    bool activateTCP;
    bool receiveBeforeSend;
    int delayAfterConnect;
    bool persistentConnectCheck;
    bool isSecure;
    void setIPmode(int mode);
    static int getIPmode();


    QList<SmartResponseConfig> smartList;


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

private:

    QUdpSocket *udpSocket;
    ThreadedTCPServer * tcp;
    ThreadedTCPServer * ssl;

    QList<TCPThread *> tcpthreadList;
    QList<PersistentConnection *> pcList;

    //TODO: eventually migrate to a list to support any number of servers.
    QList<ThreadedTCPServer *> tcpServers;
    QList<QUdpSocket *> udpServers;

};

#endif // PACKETNETWORK_H
