//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H


#include <memory>
#include <unordered_map>
#include <utility>

#include "packet.h"
#include "connections/connection.h"
#include "connections/incomingtcpconnection.h"
#include "connections/outgoingtcpconnection.h"

class ConnectionManager : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager() override;

    // Factory methods - explicit about type for future extensibility
    std::pair<quint64, IncomingTcpConnection*> createIncomingTcpConnection();
    std::pair<quint64, OutgoingTcpConnection*> createOutgoingTcpConnection();

    // Send data to connection by ID
    void send(quint64 id, const Packet &packet);

    // Close a specific connection
    void close(quint64 id);

    // Shut down all connections (called on app quit or server disable)
    void shutdownAll();

signals:
    // Forwarded with connection ID prefix
    void dataReceived(quint64 id, const Packet& packet);
    void stateChanged(quint64 id, const QString& message);
    void errorOccurred(quint64 id, const QString& errorString);
    void disconnected(quint64 id);

protected:
    std::unordered_map<quint64, std::unique_ptr<Connection>> connections;
    quint64 nextId = 1;

    // void setupConnectionSignals(Connection* conn, quint64 id);
};


#endif //CONNECTIONMANAGER_H
