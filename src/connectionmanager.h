//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H


#include <memory>
#include <unordered_map>
#include "connection.h"

class ConnectionManager : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager() override;

    // Create a persistent connection, returns unique ID
    quint64 createPersistent(const QString &host, quint16 port,
                             const Packet &initialPacket = Packet());

    // Send data to connection by ID
    void send(quint64 id, const Packet &packet);

    // Close a specific connection
    void close(quint64 id);

    // Shut down all connections (called on app quit or server disable)
    void shutdownAll();

signals:
    // Forwarded with connection ID prefix
    void dataReceived(quint64 id, const Packet &packet);
    void stateChanged(quint64 id, const QString &state);
    void errorOccurred(quint64 id, const QString &errorString);
    void disconnected(quint64 id);

private slots:
    // Internal handlers to prefix signals with ID
    void onConnectionDataReceived(const Packet &packet);
    void onConnectionStateChanged(const QString &state);
    void onConnectionError(const QString &error);
    void onConnectionDisconnected();

protected:
    std::unordered_map<quint64, std::unique_ptr<Connection>> m_connections;
    quint64 m_nextId = 1;
};


#endif //CONNECTIONMANAGER_H
