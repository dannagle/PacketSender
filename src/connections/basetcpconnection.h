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
    explicit BaseTcpConnection(std::unique_ptr<BaseTcpThread> thread,
                        QObject* parent = nullptr);

    ~BaseTcpConnection() override;

    [[nodiscard]] QString id() const override;
    [[nodiscard]] bool isConnected() const override;
    [[nodiscard]] bool isSecure() const override;
    [[nodiscard]] bool isPersistent() const override;
    [[nodiscard]] bool isIncoming() const override;

    // void send(const Packet& packet);
    // void close();

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
    bool isIncoming_ = false;

private:
    // void setupSignalConnections();
};

#endif // BASETCPCONNECTION_H
