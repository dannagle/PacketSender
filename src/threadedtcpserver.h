#ifndef THREADEDTCPSERVER_H
#define THREADEDTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>

#include "connectionmanager.h"
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

        void responsePacket(Packet packetToSend);

    protected:
        void incomingConnection(qintptr socketDescriptor);
        ConnectionManager* connectionManager;

    private:
        Packet packetReply;

#ifndef CONSOLE_BUILD
        QList<PersistentConnection *> pcList;
#endif

};

#endif // THREADEDTCPSERVER_H
