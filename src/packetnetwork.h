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
#include <QHostAddress>
#include "globals.h"
#include "tcpthread.h"
#include "packet.h"
#include "persistentconnection.h"
#include <threadedtcpserver.h>


class PacketNetwork : public QObject
{
        Q_OBJECT
    public:
        explicit PacketNetwork(QWidget *parent = 0);
        void init();

        QString debugQByteArray(QByteArray debugArray);

        QString getUDPPortString();
        QString getTCPPortString();
        QString getSSLPortString();

        void kill();
        QString responseData;
        bool sendResponse;
        bool sendSmartResponse;
        bool activateUDP;
        bool activateTCP;
        bool activateSSL;
        bool receiveBeforeSend;
        int delayAfterConnect;
        bool persistentConnectCheck;
        bool isSecure;
        void setIPmode(int mode);
        static int getIPmode();

        bool UDPListening();
        bool TCPListening();
        bool SSLListening();


        QList<SmartResponseConfig> smartList;

        static QHostAddress resolveDNS(QString hostname);
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

        QList<ThreadedTCPServer *> allTCPServers();

        QList<TCPThread *> tcpthreadList;
        QList<PersistentConnection *> pcList;

        //PS now supports any number of servers.
        QList<ThreadedTCPServer *> tcpServers;
        QList<ThreadedTCPServer *> sslServers;
        QList<QUdpSocket *> udpServers;

};

#endif // PACKETNETWORK_H
