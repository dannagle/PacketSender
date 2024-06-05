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
#include <QStandardPaths>

#include "globals.h"
#include "tcpthread.h"
#include "packet.h"

ThreadedTCPServer::ThreadedTCPServer(QObject *parent) :
    QTcpServer(parent)
{

    threads.clear();
    consoleMode = false;
    packetReply.clear();


}

bool ThreadedTCPServer::init(quint16 port, bool isEncrypted, QString ipMode)
{

    Q_UNUSED(ipMode); //actually is used via macro.

    encrypted = isEncrypted;


    tcpthreadList.clear();
#ifndef CONSOLE_BUILD
    pcList.clear();
#endif

    bool bindResult = listen(
                          IPV4_OR_IPV6
                          , port);

    QDEBUG() << "Binding" << serverPort() << bindResult;
    return bindResult;

}

void ThreadedTCPServer::responsePacket(Packet packetToSend)
{
    packetReply = packetToSend;

}

void ThreadedTCPServer::incomingConnection(qintptr socketDescriptor)
{
    QDEBUG() << "new tcp connection";

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    bool persistentConnectCheck = settings.value("persistentTCPCheck", false).toBool() && (!consoleMode);

    QDEBUGVAR(persistentConnectCheck);

    TCPThread *thread = new TCPThread(socketDescriptor, this);
    thread->isSecure = encrypted;
    thread->packetReply = packetReply;
    QDEBUGVAR(thread->isSecure);
    if (persistentConnectCheck) {
#ifndef CONSOLE_BUILD

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
#endif
    } else {

        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));


        if(consoleMode) {
            QDEBUG() << "consoleout" << connect(thread, &TCPThread::packetReceived,
                    this, &ThreadedTCPServer::outputTCPPacket);

            QDEBUG() << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(outputTCPPacket(Packet)));


        } else {
            QDEBUG() << connect(thread, SIGNAL(packetReceived(Packet)), this, SLOT(packetReceivedECHO(Packet)))
                     << connect(thread, SIGNAL(toStatusBar(QString, int, bool)), this, SLOT(toStatusBarECHO(QString, int, bool)))
                     << connect(thread, SIGNAL(packetSent(Packet)), this, SLOT(packetSentECHO(Packet)));

        }

        thread->start();

    }


}


void ThreadedTCPServer::outputTCPPacket(Packet receivePacket)
{
    QTextStream out(stdout);

    out << "\nFrom: " << receivePacket.fromIP << ", Port:" << receivePacket.fromPort;
    out << "\nResponse Time:" << QDateTime::currentDateTime().toString(DATETIMEFORMAT);

    if(!receivePacket.errorString.isEmpty()) {
        out << "\nError/Info:" << receivePacket.errorString;
    }

    if (!receivePacket.hexString.isEmpty()) {
        out << "\nResponse HEX:" << receivePacket.hexString;
        out << "\nResponse ASCII:" << receivePacket.asciiString();
    }

    out << Qt::endl;

    out.flush();


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


