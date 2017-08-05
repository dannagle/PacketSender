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

public slots:

private:
    QList<TCPThread *> threads;


};

#endif // THREADEDTCPSERVER_H
