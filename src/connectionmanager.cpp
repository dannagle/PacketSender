//
// Created by Tomas Gallucci on 3/5/26.
//

#include "connectionmanager.h"

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
{
}

ConnectionManager::~ConnectionManager()
{
    shutdownAll();  // RAII: clean up on manager destruction
}

quint64 ConnectionManager::createPersistent(const QString &host, quint16 port,
                                            const Packet &initialPacket)
{
    auto conn = std::make_unique<Connection>(host, port, initialPacket, this);

    quint64 id = m_nextId++;
    m_connections.emplace(id, std::move(conn));

    // Connect with renamed capture to avoid shadowing the signal name
    connect(m_connections[id].get(), &Connection::dataReceived,
            this, [this, connId = id](const Packet &p) { emit dataReceived(connId, p); });

    connect(m_connections[id].get(), &Connection::stateChanged,
            this, [this, connId = id](const QString &s) { emit stateChanged(connId, s); });

    connect(m_connections[id].get(), &Connection::errorOccurred,
            this, [this, connId = id](const QString &e) { emit errorOccurred(connId, e); });

    connect(m_connections[id].get(), &Connection::disconnected,
            this, [this, connId = id]() { emit disconnected(connId); });

    m_connections[id]->start();

    return id;
}

void ConnectionManager::send(quint64 id, const Packet &packet)
{
    auto it = m_connections.find(id);
    if (it != m_connections.end()) {
        it->second->send(packet);
    }
}

void ConnectionManager::close(quint64 id)
{
    auto it = m_connections.find(id);
    if (it != m_connections.end()) {
        it->second->close();  // Assuming you add close() to Connection
        m_connections.erase(it);
    }
}

void ConnectionManager::shutdownAll()
{
    // Deleting unique_ptrs triggers Connection dtors → threads close/wait
    m_connections.clear();
}

// Internal forwarders (prefix with ID)
void ConnectionManager::onConnectionDataReceived(const Packet &packet)
{
    // Slot connected per-connection with lambda above — this is placeholder if needed
}

void ConnectionManager::onConnectionStateChanged(const QString &state)
{
    // Placeholder
}

void ConnectionManager::onConnectionError(const QString &error)
{
    // Placeholder
}

void ConnectionManager::onConnectionDisconnected()
{
    // Placeholder
}

