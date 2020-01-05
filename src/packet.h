/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#ifndef PACKET_H
#define PACKET_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDebug>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QNetworkDatagram>
#include <QHash>
#include "globals.h"
#include "sendpacketbutton.h"


struct SmartResponseConfig {
    int id;
    QString ifEquals;
    QString replyWith;
    QString encoding;
    bool enabled;
};

class Packet
{
    public:
        Packet()
        {
            init();
        }
        ~Packet();

        Packet(const Packet &other);

        static QHostAddress IPV4_IPV6_ANY(QString ipMode);

        QString name;
        QString hexString;
        QString fromIP;
        QString toIP;
        QString resolvedIP;
        QString errorString;
        float repeat;
        unsigned int port;
        unsigned int fromPort;
        QString tcpOrUdp;
        unsigned int sendResponse;
        bool incoming;
        void init();
        void clear();
        bool isTCP();
        bool isSSL();
        bool isUDP();

        QDateTime timestamp;

        bool receiveBeforeSend;
        int delayAfterConnect;
        bool persistent;

        static QString ASCIITohex(QString &ascii);
        static QString hexToASCII(QString &hex);
        static QString byteArrayToHex(QByteArray data);
        static QByteArray HEXtoByteArray(QString thehex);
        static QString removeIPv6Mapping(QHostAddress ipv6);
        QByteArray getByteArray();
        QString asciiString();

        void saveToDB();


        static Packet fetchFromDB(QString thename);
        static QList<Packet> fetchAllfromDB(QString importFile);
        static bool removeFromDB(QString thename);
        static void populateTableWidgetItem(QTableWidgetItem *tItem, Packet thepacket);
        static Packet fetchTableWidgetItemData(QTableWidgetItem *tItem);
        static SmartResponseConfig fetchSmartConfig(int num, QString importFile);
        static QByteArray smartResponseMatch(QList<SmartResponseConfig> smartList, QByteArray data);
        static QByteArray encodingToByteArray(QString encoding, QString data);

        static const int PACKET_NAME;
        static const int PACKET_HEX;
        static const int FROM_IP;
        static const int FROM_PORT;
        static const int TCP_UDP;
        static const int TO_PORT;
        static const int TO_IP;

        static const int TIMESTAMP;
        static const int DATATYPE;
        static const int REPEAT;
        static const int INCOMING;


        bool operator()(const Packet* a, const Packet* b) const;

        SendPacketButton * getSendButton(QTableWidget *parent);
        QIcon getIcon();
        static void sortByName(QList<Packet> &packetList);
        static void sortByTime(QList<Packet> &packetList);
        static float oneDecimal(float value);
        static QString macroSwap(QString data);
        static QByteArray ExportJSON(QList<Packet> packetList);
        static QList<Packet> ImportJSON(QByteArray data);
        void static removeFromDBList(QStringList nameList);
        static void setBoldItem(QTableWidgetItem *tItem, Packet thepacket);
private:
        static int hexToInt(QChar hex);
};

Q_DECLARE_METATYPE(Packet)

#endif // PACKET_H
