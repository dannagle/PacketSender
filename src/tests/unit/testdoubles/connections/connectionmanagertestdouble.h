//
// Created by Tomas Gallucci on 6/28/26.
//

#ifndef CONNECTIONMANAGERTESTDOUBLE_H
#define CONNECTIONMANAGERTESTDOUBLE_H
#include "connectionmanager.h"
#include "utils/calltracker.h"

class ConnectionManagerTestDouble: public ConnectionManager, public CallTracker
{
    Q_OBJECT

public:
    quint64 getCurrentNextIdValue() const
    {
        return nextId;
    }

    std::unordered_map<quint64, std::unique_ptr<Connection>>& getMap()
    {
        return connections;
    }

protected:
    void setupConnectionSignals(Connection* conn, quint64 id) override
    {
        recordCall(SETUP_SIGNAL_CONNECTIONS());
        ConnectionManager::setupConnectionSignals(conn, id);
    }

};

#endif //CONNECTIONMANAGERTESTDOUBLE_H
