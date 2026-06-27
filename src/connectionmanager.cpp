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


void ConnectionManager::send(quint64 id, const Packet &packet)
{
    // auto it = m_connections.find(id);
    // if (it != m_connections.end()) {
    //     it->second->send(packet);
    // }
}

void ConnectionManager::close(quint64 id)
{
    // auto it = m_connections.find(id);
    // if (it != m_connections.end()) {
    //     it->second->close();  // Assuming you add close() to Connection
    //     m_connections.erase(it);
    // }
}

void ConnectionManager::shutdownAll()
{
    // Deleting unique_ptrs triggers Connection dtors → threads close/wait
    connections.clear();
}
