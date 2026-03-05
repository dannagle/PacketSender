//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef CONNECTION_H
#define CONNECTION_H


#pragma once

#include <QObject>
#include <QString>
#include <QUuid>

/**
 * @brief RAII-style wrapper for a persistent connection.
 *        Initially minimal; will later own a TCPThread or similar.
 */
class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(const QString &host, quint16 port, QObject *parent = nullptr);
    ~Connection() override;

    QString id() const;

    // Placeholder for future API
    // void send(const QByteArray &data);
    // etc.

private:
    QString m_id;
    // QString m_host;       // uncomment later if needed for reconnect
    // quint16 m_port = 0;
    // TCPThread *m_thread = nullptr;  // or std::unique_ptr<TCPThread> — add in next step
};


#endif //CONNECTION_H
