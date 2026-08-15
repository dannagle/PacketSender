//
// Created by Tomas Gallucci on 3/5/26.
//

#include "connectionmanager.h"

#include "connections/incomingtcpconnection.h"

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
{
}

ConnectionManager::~ConnectionManager()
{
    shutdownAll();  // RAII: clean up on manager destruction
}

bool ConnectionManager::hasConnection(quint64 id)
{
    return connections.find(id) != connections.end();
}

void ConnectionManager::setupConnectionSignals(Connection* conn, quint64 id)
{
    if (!conn) return;

    connect(conn, &Connection::dataReceived,
            this, [this, id](const Packet& p) {
                emit dataReceived(id, p);
            });
    connect(conn, &Connection::packetSent,
            this, [this, id](const Packet& p) {
                emit packetSent(id, p);
            });

    connect(conn, &Connection::stateChanged,
            this, [this, id](const QString& msg) {
                emit stateChanged(id, msg);
            });

    connect(conn, &Connection::errorOccurred,
            this, [this, id](const QString& err) {
                emit errorOccurred(id, err);
            });

    connect(conn, &Connection::disconnected,
            this, [this, id]() {
                if (
                    const auto it = connections.find(id);
                    it != connections.end() && it->second->isConnected())
                {
                    // QDEBUG() << "calling close() on connection in ConnectionManager::disconnect()";
                    QDEBUG() << "removing connection from map in ConnectionManager::disconnect()";
                    it->second->close();
                    connections.erase(id);
                    emit disconnected(id);
                }
            });
}

std::unique_ptr<OutgoingTcpConnection> ConnectionManager::createOutgoingTcpConnectionObject()
{
    return std::make_unique<OutgoingTcpConnection>(this);
}

std::unique_ptr<IncomingTcpConnection> ConnectionManager::createIncomingTcpConnectionObject()
{
    return std::make_unique<IncomingTcpConnection>(this);
}

std::pair<quint64, IncomingTcpConnection*> ConnectionManager::createIncomingTcpConnection()
{
    auto conn = createIncomingTcpConnectionObject();
    quint64 id = nextId++;

    IncomingTcpConnection* rawPtr = conn.get();
    connections[id] = std::move(conn);
    setupConnectionSignals(connections[id].get(), id);

    return {id, rawPtr};
}

std::pair<quint64, OutgoingTcpConnection*> ConnectionManager::createOutgoingTcpConnection()
{
    auto conn = createOutgoingTcpConnectionObject();

    quint64 id = nextId++;

    OutgoingTcpConnection* rawPtr = conn.get();
    connections[id] = std::move(conn);
    setupConnectionSignals(connections[id].get(), id);

    return {id, rawPtr};
}

void ConnectionManager::send(quint64 id, const Packet &packet)
{
    auto it = connections.find(id);
    if (it != connections.end()) {
        it->second->send(packet);
    }
}

void ConnectionManager::close(quint64 id)
{
    QMutexLocker locker(&mutex);
    const auto it = connections.find(id);

    if (it == connections.end()) {
        QDEBUG() << "returning early from ConnectionManager::close()";
        return;                     // early return — very clear
    }

    if (it->second->isConnected()) {
        QDEBUG() << "calling close on thread in ConnectionManager::close()";
        it->second->close();
        QDEBUG() << "called close on thread in ConnectionManager::close()";
    }

    QDEBUG() << "about to erase thread in ConnectionManager::close()";
    connections.erase(it);
    QDEBUG() << "erased thread in ConnectionManager::close()";
}

void ConnectionManager::shutdownAll()
{
    // Deleting unique_ptrs triggers Connection dtors → threads close/wait
    connections.clear();
}
