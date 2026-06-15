//
// Created by Tomas Gallucci on 6/14/26.
//

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <memory>
#include "packet.h"
#include "basetcpthread.h"

class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(std::unique_ptr<BaseTcpThread> thread,
                        QObject* parent = nullptr);

    ~Connection() override;

    [[nodiscard]] QString id() const;
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isSecure() const;
    // [[nodiscard]] bool isPersistent() const;
    // [[nodiscard]] bool isIncoming() const;

    // void send(const Packet& packet);
    // void close();

    signals:
    void dataReceived(const Packet& packet);
    void stateChanged(const QString& message);
    void errorOccurred(const QString& errorString);
    void disconnected();

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
    void assignUniqueId();
    std::optional<QString> id_;
};

#endif // CONNECTION_H
