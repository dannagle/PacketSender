/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */
#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QThread>
#include <QSslSocket>
#include "packet.h"

class TCPThread : public QThread
{
        Q_OBJECT

    public:
        TCPThread(int socketDescriptor, QObject *parent);
        TCPThread(Packet sendPacket, QObject *parent);

        // NEW constructor for Connection-managed persistent client
        TCPThread(const QString &host, quint16 port,
            const Packet &initialPacket = Packet(),
            QObject *parent = nullptr);

        void sendAnother(Packet sendPacket);
        static void loadSSLCerts(QSslSocket *sock, bool allowSnakeOil);

        void run();
        bool sendFlag;
        bool incomingPersistent;
        bool closeRequest;
        bool isSecure;
        bool isEncrypted();
        Packet packetReply;
        bool consoleMode;
        [[nodiscard]] bool isValid() const;

    signals:
        void error(QSslSocket::SocketError socketError);

        void packetReceived(Packet sendpacket);
        void toStatusBar(const QString & message, int timeout = 0, bool override = false);
        void packetSent(Packet sendpacket);
        void connectStatus(QString message);

    public slots:
        void sendPersistant(Packet sendpacket);

        void closeConnection();
    private slots:
        void wasdisconnected();

        void onConnected();
        void onSocketError(QAbstractSocket::SocketError socketError);
        void onStateChanged(QAbstractSocket::SocketState state);

    private:
        int socketDescriptor;
        QString text;
        Packet sendPacket;
        void init();
        void writeResponse(QSslSocket *sock, Packet tcpPacket);
        void wireupSocketSignals();
        QSslSocket * clientConnection;
        bool insidePersistent;

        void persistentConnectionLoop();

        QString host;
        quint16 port = 0;
        bool m_managedByConnection = false;  // flag to skip deleteLater() in run()
};

#endif // TCPTHREAD_H
