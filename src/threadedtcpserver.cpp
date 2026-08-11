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
#include "packet.h"
#include "connections/persistentconnectionwiring.h"

ThreadedTCPServer::ThreadedTCPServer(ConnectionManager* manager, QObject* parent)
    : QTcpServer(parent)
{
    connectionManager = manager;

    consoleMode = false;
    packetReply.clear();
}

bool ThreadedTCPServer::init(const quint16 port, const bool isEncrypted, const QString& ipMode)
{
    Q_UNUSED(ipMode); //actually is used via macro.

    encrypted = isEncrypted;

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

    if (!connectionManager) {
        QDEBUG() << "connectionManager is null";
        return;
    }

    // Decide whether this should be a persistent (GUI) connection
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    const bool isPersistentConnection =
        settings.value("persistentTCPCheck", false).toBool() && !consoleMode;

    QDEBUGVAR(isPersistentConnection);

    auto [id, conn] = connectionManager->createIncomingTcpConnection();

#ifndef CONSOLE_BUILD
    // ---------------------------------------------------------------
    // Persistent GUI path – create the dedicated window and wire it
    // ---------------------------------------------------------------
    if (isPersistentConnection) {
        PersistentConnection *pcWindow = new PersistentConnection();
        pcWindow->initWithConnection(id, serverPort(), encrypted);
        PersistentConnectionWiring::setupPersistentWindowConnections(pcWindow, connectionManager, id);

        pcWindow->show();
        pcList.append(pcWindow);   // keep the window alive
    }
#endif

    // Hand the socket over to the connection object
    conn->receiveData(socketDescriptor, encrypted, isPersistentConnection);
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


