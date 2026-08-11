//
// Created by Tomas Gallucci on 8/11/26.
//

#ifndef PERSISTENTCONNECTIONWIRING_H
#define PERSISTENTCONNECTIONWIRING_H
#include "connectionmanager.h"
#include "persistentconnection.h"


class PersistentConnectionWiring
{
 public:
#ifndef CONSOLE_BUILD
    static void setupPersistentWindowConnections(PersistentConnection *pcWindow, ConnectionManager *manager, quint64 id);
#endif
};


#endif //PERSISTENTCONNECTIONWIRING_H
