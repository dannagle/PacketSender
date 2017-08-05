#ifndef THREADEDTCPSERVER_H
#define THREADEDTCPSERVER_H

#include <QObject>
#include <QTcpServer>

class ThreadedTCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ThreadedTCPServer( QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:

public slots:
};

#endif // THREADEDTCPSERVER_H
