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

ThreadedTCPServer::ThreadedTCPServer(ConnectionManager* manager, QObject* parent)
    : QTcpServer(parent)
{
    connectionManager = manager;

    consoleMode = false;
    packetReply.clear();
}

void ThreadedTCPServer::setupGlobalLogging()
{
    static bool done = false;
    if (done)
        return;
    done = true;

    if (!consoleMode) {
        connect(connectionManager, &ConnectionManager::dataReceived,
                this, [this](quint64, const Packet &p) {
                    packetReceivedECHO(p);
                });
        connect(connectionManager, &ConnectionManager::packetSent,
                this, [this](quint64, const Packet &p) {
                    packetSentECHO(p);
                });
    } else {
        connect(connectionManager, &ConnectionManager::dataReceived,
                this, [this](quint64, const Packet &p) {
                    outputTCPPacket(p);
                });
        connect(connectionManager, &ConnectionManager::packetSent,
                this, [this](quint64, const Packet &p) {
                    outputTCPPacket(p);
                });
    }
}

#ifndef CONSOLE_BUILD
void ThreadedTCPServer::setupPersistentWindowConnections(PersistentConnection *pcWindow, const quint64 id)
{
    // Window → Manager
    connect(pcWindow, &PersistentConnection::persistentPacketSend,
            this, [this, id](const Packet &p) {
                connectionManager->send(id, p);
            });

    connect(pcWindow, &PersistentConnection::closeConnection,
            this, [this, id]() {
                connectionManager->close(id);
            });

    // Manager → Window (filtered by id)
    connect(connectionManager, &ConnectionManager::stateChanged,
            pcWindow, [pcWindow, id](quint64 cid, const QString &msg) {
                if (cid == id)
                    pcWindow->statusReceiver(msg);
            });

    connect(connectionManager, &ConnectionManager::packetSent,
            pcWindow, [pcWindow, id](quint64 cid, const Packet &p) {
                if (cid == id)
                    pcWindow->packetSentSlot(p);
            });

    connect(connectionManager, &ConnectionManager::dataReceived,
            pcWindow, [pcWindow, id](quint64 cid, const Packet &p) {
                if (cid == id)
                    pcWindow->packetReceivedSlot(p);
            });

    connect(connectionManager, &ConnectionManager::disconnected,
            pcWindow, [pcWindow, id](quint64 cid) {
                if (cid == id)
                    pcWindow->socketDisconnected();
            });
}
#endif

bool ThreadedTCPServer::init(const quint16 port, const bool isEncrypted, const QString& ipMode)
{
    Q_UNUSED(ipMode); //actually is used via macro.

    encrypted = isEncrypted;

#ifndef CONSOLE_BUILD
    pcList.clear();
#endif

    setupGlobalLogging();

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
        setupPersistentWindowConnections(pcWindow, id);

        pcWindow->show();
        pcList.append(pcWindow);   // keep the window alive
    }
#endif

    // Hand the socket over to the connection object
    conn->receiveData(socketDescriptor, encrypted, isPersistentConnection);
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

    out << ENDL;

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


