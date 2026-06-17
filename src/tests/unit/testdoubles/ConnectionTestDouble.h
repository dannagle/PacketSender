//
// Created by Tomas Gallucci on 6/16/26.
//

#ifndef CONNECTIONTESTDOUBLE_H
#define CONNECTIONTESTDOUBLE_H
#include <QObject>

#include "connections/connection.h"

class ConnectionTestDouble : public Connection
{
    Q_OBJECT
public:
    bool connected = false;
    bool encrypted = false;
    bool persistent = false;
    bool incoming = false;

    [[nodiscard]] bool isConnected() const override { return connected; };
    [[nodiscard]] bool isSecure() const override { return encrypted; };
    [[nodiscard]] bool isPersistent() const override { return persistent; };
    [[nodiscard]] bool isIncoming() const override { return incoming; };

};

#endif //CONNECTIONTESTDOUBLE_H
