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
#include "globals.h"
#include "sendpacketbutton.h"


class Packet
{
public:
    Packet(){init(); }
    ~Packet();

    Packet(const Packet &other);

    QString name;
    QString hexString;
    QString fromIP;
    QString toIP;
    QString errorString;
    unsigned int repeat;
    unsigned int port;
    unsigned int fromPort;
    QString tcpOrUdp;
    unsigned int sendResponse;
    void init();
    void clear();
    bool isTCP();
    QDateTime timestamp;

    bool receiveBeforeSend;
    int delayAfterConnect;
    bool persistent;

    static QString ASCIITohex(QString &ascii);
    static QString hexToASCII(QString &hex);
    static QString byteArrayToHex(QByteArray data);
    static QByteArray HEXtoByteArray(QString thehex);
    QByteArray getByteArray();
    QString asciiString();

    void saveToDB();


    static Packet fetchFromDB(QString thename);
    static QList<Packet> fetchAllfromDB(QString importFile);
    static bool removeFromDB(QString thename);
    static void populateTableWidgetItem(QTableWidgetItem *tItem, Packet thepacket);
    static Packet fetchTableWidgetItemData(QTableWidgetItem *tItem);

    static const int PACKET_NAME;
    static const int PACKET_HEX;
    static const int FROM_IP;
    static const int FROM_PORT;
    static const int TCP_UDP;
    static const int TO_PORT;
    static const int TO_IP;
    static const int SEND_RESPONSE;
    static const int TIMESTAMP;
    static const int DATATYPE;
    static const int REPEAT;

    bool operator()(const Packet* a, const Packet* b) const;

    SendPacketButton * getSendButton(QTableWidget *parent);
    QIcon getIcon();
    static void sortByName(QList<Packet> &packetList);
    static void sortByTime(QList<Packet> &packetList);
private:
    static int hexToInt(QChar hex);


};

Q_DECLARE_METATYPE(Packet)

#endif // PACKET_H
