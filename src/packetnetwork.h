/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
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
#include "tcpthread.h"
#include "packet.h"
#include "connectionmanager.h"
#ifndef CONSOLE_BUILD
#include "persistentconnection.h"
#endif
#include <threadedtcpserver.h>
#include <QSettings>
#include "dtlsthread.h"
#include "dtlsserver.h"


struct MulticastMembership {
        QString address;
        QNetworkInterface iface;

        bool operator==(const MulticastMembership &other) const
        {
                return address == other.address && iface.index() == other.iface.index();
        }

        bool operator!=(const MulticastMembership &other) const
        {
                return !(*this == other);
        }
};

class PacketNetwork : public QObject
{
        Q_OBJECT
    public:
        QString keyPath;
        QString certPath;
        explicit PacketNetwork(QObject *parent = nullptr);
        void init();

        std::vector<QString> getCmdInput(Packet sendpacket, QSettings &settings);

        QString debugQByteArray(QByteArray debugArray);

        QString getDTLSPortString();
        QString getUDPPortString();
        QString getTCPPortString();
        QString getSSLPortString();

        QList<int> getDTLSPortsBound();
        QList<int> getUDPPortsBound();
        QList<int> getTCPPortsBound();
        QList<int> getSSLPortsBound();

        QStringList multicastStringList() const;

        bool consoleMode;

        void kill();
        QString responseData;
        bool sendResponse;
        bool sendSmartResponse;
        bool activateDTLS;
        bool activateUDP;
        bool activateTCP;
        bool activateSSL;
        bool receiveBeforeSend;
        bool translateMacroSend;
        int delayAfterConnect;
        bool persistentConnectCheck;
        bool isSecure;
        void setIPmode(int mode);
        static QString getIPmode();

        bool DTLSListening();
        bool UDPListening();
        bool TCPListening();
        bool SSLListening();

        bool IPv6Enabled();
        bool IPv4Enabled();

        QNetworkAccessManager * http;

        QList<SmartResponseConfig> smartList;

        static QHostAddress resolveDNS(QString hostname);

        static bool isMulticast(QString ip);

        bool joinMulticast(const QString& address, const QNetworkInterface &iface);
        [[nodiscard]] bool canSendMulticast(const QString &address) const;
        void leaveMulticast();
        bool leaveMulticast(const QString &address, const QNetworkInterface &iface); // leave single multicast group on a given interface
        bool leaveMulticast(const QString &address, const QString &ifaceName);

        QUdpSocket * findMulticast(const QString &multicast) const;
        static bool DTLSisSupported();
        QList<DtlsServer *> dtlsServers;

        ConnectionManager* getConnectionManager() { return &connectionManager; };


    signals:
        void packetReceived(Packet sendpacket);
        void toStatusBar(const QString & message, int timeout = 0, bool override = false);
        void packetSent(Packet sendpacket);


public slots:
        void packetReceivedECHO(Packet sendpacket);
        void toStatusBarECHO(const QString & message, int timeout = 0, bool override = false);
        void packetSentECHO(Packet sendpacket);
        void outputPacket(Packet receivePacket);
        void readPendingDatagrams();
        void disconnected();
        void packetToSend(Packet sendpacket);
        void on_twoVerify_StateChanged();

protected:
        ConnectionManager connectionManager;

private slots:
        void httpFinished(QNetworkReply* pReply);
        void httpError(QNetworkRequest* pReply);
        void sslErrorsSlot(QNetworkReply *reply, const QList<QSslError> &errors);


private:
        //mapping of joined multicast groups
        //format is 239.255.120.19:5009, 239.255.120.23:5009
        QList<MulticastMembership> joinedMulticast;   // group address → interface

        QList<ThreadedTCPServer *> allTCPServers();

        QList<QNetworkAccessManager *> httpList;
        QList<Dtlsthread *> dtlsthreadList;

        QList<TCPThread *> tcpthreadList;
#ifdef CONSOLE_BUILD
        QList<void *> pcList;
#else
        QList<PersistentConnection *> pcList;
#endif
        //PS now supports any number of servers.
        QList<ThreadedTCPServer *> tcpServers;
        QList<ThreadedTCPServer *> sslServers;
        QList<QUdpSocket *> udpServers;



};

#endif // PACKETNETWORK_H
