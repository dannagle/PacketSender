//
// Created by Tomas Gallucci on 6/28/26.
//

#ifndef CONNECTIONMANAGERTESTDOUBLE_H
#define CONNECTIONMANAGERTESTDOUBLE_H
#include "connectionmanager.h"

class ConnectionManagerTestDouble: public ConnectionManager
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

};

#endif //CONNECTIONMANAGERTESTDOUBLE_H
