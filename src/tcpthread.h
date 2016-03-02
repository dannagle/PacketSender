/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include "packet.h"

class TCPThread : public QThread
{
    Q_OBJECT

public:
    TCPThread(int socketDescriptor, QObject *parent);
    TCPThread(Packet sendPacket, QObject *parent);
    void sendAnother(Packet sendPacket);

    void run();
    bool sendFlag;
    bool incomingPersistent;

signals:
    void error(QTcpSocket::SocketError socketError);

    void packetReceived(Packet sendpacket);
    void toStatusBar(const QString & message, int timeout = 0, bool override = false);
    void packetSent(Packet sendpacket);
    void connectStatus(QString message);

public slots:
    void sendPersistant(Packet sendpacket);

    void closeConnection();
private slots:
    void wasdisconnected();

private:
    int socketDescriptor;
    QString text;
    Packet sendPacket;
    Packet sendPacketPersistent;
    void init();
    void writeResponse(QTcpSocket *sock, Packet tcpPacket);
    QTcpSocket * clientConnection;

};

#endif // TCPTHREAD_H
