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
                emit disconnected(id);
            });
}

std::pair<quint64, IncomingTcpConnection*> ConnectionManager::createIncomingTcpConnection()
{
    auto conn = std::make_unique<IncomingTcpConnection>(this);

    quint64 id = nextId++;
    setupConnectionSignals(conn.get(), id);

    IncomingTcpConnection* rawPtr = conn.get();
    connections[id] = std::move(conn);

    return {id, rawPtr};
}

std::pair<quint64, OutgoingTcpConnection*> ConnectionManager::createOutgoingTcpConnection()
{
    auto conn = std::make_unique<OutgoingTcpConnection>(this);

    quint64 id = nextId++;
    setupConnectionSignals(conn.get(), id);

    OutgoingTcpConnection* rawPtr = conn.get();
    connections[id] = std::move(conn);

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
    auto it = connections.find(id);
    if (it != connections.end()) {
        it->second->close();
        connections.erase(it);
    }
}

void ConnectionManager::shutdownAll()
{
    // Deleting unique_ptrs triggers Connection dtors → threads close/wait
    connections.clear();
}
