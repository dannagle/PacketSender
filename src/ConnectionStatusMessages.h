//
// Created by Tomas Gallucci on 7/4/26.
//

#ifndef CONNECTIONSTATUSMESSAGES_H
#define CONNECTIONSTATUSMESSAGES_H
#include <QString>

class ConnectionStatusMessages
{
public:
    static QString INCOMING_CONNECTION_ACCEPTED() { return QStringLiteral("Incoming connection accepted"); }
    static QString ERROR_NO_SOCKET_AVAILABLE() { return QStringLiteral("Error: No socket available"); }
    static QString ERROR_SOCKET_NOT_CONNECTED() { return QStringLiteral("Error: Socket not connected"); }
    static QString SENDING_DATA() { return QStringLiteral("Sending data: "); }
    static QString DISCONNECTED() { return QStringLiteral("Disconnected"); }
    static QString SSL_CONNECTED() { return QStringLiteral("SSL Connected"); }
    static QString SSL_HANDSHAKE_FAILED() { return QStringLiteral("SSL Handshake Failed"); }
    static QString CONNECTED() { return QStringLiteral("Connected"); }
    static QString COULD_NOT_CONNECT() { return QStringLiteral("Could not connect."); }
    static QString CONNECTED_AND_IDLE() { return QStringLiteral("Connected and idle."); }
    static QString WAITING_FOR_DATA_BEFORE_SEND() { return QStringLiteral("Waiting for data before send"); }
};
#endif //CONNECTIONSTATUSMESSAGES_H
