#include "threadedtcpserver.h"

#include <QDebug>
#include <QSslSocket>
#include <QHostAddress>
#include <QSslConfiguration>

#include <QDesktopServices>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QtGlobal>
#include <QSettings>

#include "globals.h"
#include "tcpthread.h"
#include "packet.h"

ThreadedTCPServer::ThreadedTCPServer(QObject *parent) :
    QTcpServer(parent)
{

    threads.clear();


}

bool ThreadedTCPServer::init(quint16 port, bool isEncrypted, int ipMode)
{

    Q_UNUSED(ipMode); //actually is used via macro.

    encrypted = isEncrypted;


    tcpthreadList.clear();
    pcList.clear();


    bool bindResult = listen(
                IPV4_OR_IPV6
                , port);

    QDEBUG() << "Binding" << serverPort() << bindResult;
    return bindResult;

}

void ThreadedTCPServer::incomingConnection(qintptr socketDescriptor)
{
    QDEBUG() << "new tcp connection";

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    bool persistentConnectCheck = settings.value("persistentConnectCheck", false).toBool();

    TCPThread *thread = new TCPThread(socketDescriptor, this);
    thread->isSecure = encrypted;
    QDEBUGVAR(thread->isSecure);
    if(persistentConnectCheck) {
        PersistentConnection * pcWindow = new PersistentConnection();
        thread->incomingPersistent = true;
        pcWindow->initWithThread(thread, serverPort());

        connect(pcWindow->thread, SIGNAL(finished()), pcWindow, SLOT(socketDisconnected()));

        QDEBUG() << connect(pcWindow->thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(pcWindow->thread, SIGNAL(toStatusBar(QString,int,bool)), this, SLOT(toStatusBarECHO(QString,int,bool)))
                 << connect(pcWindow->thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));
        QDEBUG() << connect(pcWindow->thread, SIGNAL(destroyed()),pcWindow, SLOT(socketDisconnected()));


        pcWindow->show();

        //Prevent Qt from auto-destroying these windows.
        //TODO: Use a real connection manager.
        pcList.append(pcWindow);

        //TODO: Use a real connection manager.
        //prevent Qt from auto-destorying this thread while it tries to close.
        tcpthreadList.append(pcWindow->thread);

    } else {

        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString,int,bool)), this, SLOT(toStatusBarECHO(QString,int,bool)))
                 << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));

        thread->start();

    }


}


void ThreadedTCPServer::packetReceivedECHO(Packet sendpacket)
{
    emit packetReceived(sendpacket);
}

void ThreadedTCPServer::toStatusBarECHO(const QString &message, int timeout, bool override)
{
    emit toStatusBar(message, timeout, override);

}

void ThreadedTCPServer::packetSentECHO(Packet sendpacket)
{
    emit packetSent(sendpacket);

}


