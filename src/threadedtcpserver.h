#ifndef THREADEDTCPSERVER_H
#define THREADEDTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>

#include "tcpthread.h"
#include "persistentconnection.h"

class ThreadedTCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ThreadedTCPServer( QObject *parent = nullptr);
    bool encrypted;

    bool init(quint16 port, bool isEncrypted, int ipMode);
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



private:
    QList<TCPThread *> threads;


    QList<TCPThread *> tcpthreadList;
    QList<PersistentConnection *> pcList;


};

#endif // THREADEDTCPSERVER_H
