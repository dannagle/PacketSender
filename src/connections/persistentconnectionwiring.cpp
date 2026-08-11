//
// Created by Tomas Gallucci on 8/11/26.
//

#ifndef CONSOLE_BUILD
#include "persistentconnectionwiring.h"
#include "connectionmanager.h"
#include "persistentconnection.h"

void PersistentConnectionWiring::setupPersistentWindowConnections(PersistentConnection* pcWindow, ConnectionManager *manager, quint64 id)
{
    // Window → Manager
    QObject::connect(pcWindow, &PersistentConnection::persistentPacketSend,
                     manager, [manager, id](const Packet &p) {
                         manager->send(id, p);
                     });

    QObject::connect(pcWindow, &PersistentConnection::closeConnection,
                     manager, [manager, id]() {
                         manager->close(id);
                     });

    // Manager → Window (filtered by id)
    QObject::connect(manager, &ConnectionManager::stateChanged,
                     pcWindow, [pcWindow, id](quint64 cid, const QString &msg) {
                         if (cid == id)
                             pcWindow->statusReceiver(msg);
                     });

    QObject::connect(manager, &ConnectionManager::packetSent,
                     pcWindow, [pcWindow, id](quint64 cid, const Packet &p) {
                         if (cid == id)
                             pcWindow->packetSentSlot(p);
                     });

    QObject::connect(manager, &ConnectionManager::dataReceived,
                     pcWindow, [pcWindow, id](quint64 cid, const Packet &p) {
                         if (cid == id)
                             pcWindow->packetReceivedSlot(p);
                     });

    QObject::connect(manager, &ConnectionManager::disconnected,
                     pcWindow, [pcWindow, id](quint64 cid) {
                         if (cid == id)
                             pcWindow->socketDisconnected();
                     });
}
#endif
