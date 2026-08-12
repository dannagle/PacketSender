//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef BASETCPCONNECTION_H
#define BASETCPCONNECTION_H

#include <QObject>
#include <memory>

#include "connection.h"
#include "../packet.h"
#include "../basetcpthread.h"

class BaseTcpConnection : public Connection
{
    Q_OBJECT

public:
    explicit BaseTcpConnection(QObject* parent = nullptr);

    ~BaseTcpConnection() override;

    [[nodiscard]] bool isConnected() const override;
    [[nodiscard]] bool isSecure() const override;
    [[nodiscard]] bool isPersistent() const override;
    [[nodiscard]] bool isIncoming() const override;
    void moveSocketToWorkerThread();

    void send(const Packet& packet) override;
    void receiveData(int socketDescriptor, bool isSecure = false, bool persistent = false) override;
    void close() override;

protected:
    std::unique_ptr<BaseTcpThread> thread_;
    virtual void terminateConnection();

    void setupSignalConnections() override;
    State stateFromMessage(const QString& msg) const;
    virtual void setState(State state);
};

#endif // BASETCPCONNECTION_H
