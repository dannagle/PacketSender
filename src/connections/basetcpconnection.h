//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef BASETCPCONNECTION_H
#define BASETCPCONNECTION_H

#include <QObject>
#include <memory>

#include "connection.h"
#include "incomingtcpthread.h"
#include "../packet.h"
#include "../basetcpthread.h"

class BaseTcpConnection : public Connection
{
    Q_OBJECT

public:
    explicit BaseTcpConnection(QObject* parent = nullptr);

    ~BaseTcpConnection() override = default;

    [[nodiscard]] bool isConnected() const override;
    [[nodiscard]] bool isSecure() const override;
    [[nodiscard]] bool isPersistent() const override;
    [[nodiscard]] bool isIncoming() const override;

    void send(const Packet& packet) override;
    void close() override;

    // signals:
    // void dataReceived(const Packet& packet);
    // void stateChanged(const QString& message);
    // void errorOccurred(const QString& errorString);
    // void disconnected();

private slots:
    // void onThreadPacketReceived(const Packet& p);
    // void onThreadPacketSent(const Packet& p);
    // void onThreadConnectionStatus(const QString& msg);
    // void onThreadError(const QString& errorMsg);

protected:
    std::unique_ptr<BaseTcpThread> thread_;
    virtual void terminateConnection();

private:
    // void setupSignalConnections();
};

#endif // BASETCPCONNECTION_H
