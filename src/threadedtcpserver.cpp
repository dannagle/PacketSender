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

bool ThreadedTCPServer::init(quint16 port, bool isEncrypted, QString ipMode)
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
    bool persistentConnectCheck = settings.value("persistentTCPCheck", false).toBool();

    QDEBUGVAR(persistentConnectCheck);

    TCPThread *thread = new TCPThread(socketDescriptor, this);
    thread->isSecure = encrypted;
    QDEBUGVAR(thread->isSecure);
    if (persistentConnectCheck) {
        PersistentConnection * pcWindow = new PersistentConnection();
        thread->incomingPersistent = true;
        pcWindow->initWithThread(thread, serverPort());


        connect(thread, SIGNAL(finished()), pcWindow, SLOT(socketDisconnected()));


        QDEBUG() << ": thread Connection attempt " <<
                 connect(pcWindow, SIGNAL(persistentPacketSend(Packet)), thread, SLOT(sendPersistant(Packet)))
                 << connect(pcWindow, SIGNAL(closeConnection()), thread, SLOT(closeConnection()))
                 << connect(thread, SIGNAL(connectStatus(QString)), pcWindow, SLOT(statusReceiver(QString)))
                 << connect(thread, SIGNAL(packetSent(Packet)), pcWindow, SLOT(packetSentSlot(Packet)));

        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                 << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));



        thread->start();

        pcWindow->show();

        //Prevent Qt from auto-destroying these windows.
        //TODO: Use a real connection manager.
        pcList.append(pcWindow);

        //TODO: Use a real connection manager.
        //prevent Qt from auto-destorying this thread while it tries to close.
        tcpthreadList.append(thread);

    } else {

        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                 << connect(thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
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


