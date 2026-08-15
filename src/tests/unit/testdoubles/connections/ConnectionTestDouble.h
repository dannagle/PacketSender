//
// Created by Tomas Gallucci on 6/16/26.
//

#ifndef CONNECTIONTESTDOUBLE_H
#define CONNECTIONTESTDOUBLE_H
#include <QObject>

#include "connections/connection.h"
#include "utils/calltracker.h"

class ConnectionTestDouble : public Connection, public CallTracker
{
    Q_OBJECT
public:
    bool connected = false;
    bool encrypted = false;
    bool persistent_ = false;
    bool incoming = false;

    [[nodiscard]] bool isConnected() const override { return connected; };
    [[nodiscard]] bool isSecure() const override { return encrypted; };
    [[nodiscard]] bool isPersistent() const override { return persistent_; };
    [[nodiscard]] bool isIncoming() const override { return incoming; };

    [[nodiscard]] QString callGetClassName() const
    {
        return Connection::getClassName();
    }

    void send(const Packet& packet) override
    {
        const auto errorMessage = "Unsupported Operation: "
        + getClassName() + " cannot send Packet";
        throw std::runtime_error(errorMessage.toUtf8());
    }

    void receiveData(const int port, bool isSecure, bool persistent) override
    {
        Q_UNUSED(port);
        Q_UNUSED(isSecure);
        Q_UNUSED(persistent);

        const auto errorMessage = "Unsupported Operation: "
            + getClassName() + " cannot receive data";
        throw std::runtime_error(errorMessage.toUtf8());
    }

    void close() override
    {

    }

protected:
    void setupSignalConnections() override
    {

    }
};

#endif //CONNECTIONTESTDOUBLE_H
