#ifndef THREADEDTCPSERVER_H
#define THREADEDTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>

#include "connectionmanager.h"
#include "tcpthread.h"
#ifndef CONSOLE_BUILD
#include "persistentconnection.h"
#endif
class ThreadedTCPServer : public QTcpServer
{
        Q_OBJECT
    public:
        explicit ThreadedTCPServer(ConnectionManager* manager, QObject *parent = nullptr);
        bool encrypted;
        bool consoleMode;

        bool init(quint16 port, bool isEncrypted, const QString& ipMode);
        void setupGlobalLogging();

        #ifndef CONSOLE_BUILD
            void setupPersistentWindowConnections(PersistentConnection *pcWindow, quint64 id);
        #endif

        void responsePacket(Packet packetToSend);

    protected:
        void incomingConnection(qintptr socketDescriptor);
        ConnectionManager* connectionManager;

    signals:
        void packetReceived(Packet sendpacket);
        void toStatusBar(const QString & message, int timeout = 0, bool override = false);
        void packetSent(Packet sendpacket);


    public slots:
        void packetReceivedECHO(Packet sendpacket);
        void toStatusBarECHO(const QString & message, int timeout = 0, bool override = false);
        void packetSentECHO(Packet sendpacket);
        void outputTCPPacket(Packet receivePacket);


    private:
        Packet packetReply;

#ifndef CONSOLE_BUILD
        QList<PersistentConnection *> pcList;
#endif

};

#endif // THREADEDTCPSERVER_H
