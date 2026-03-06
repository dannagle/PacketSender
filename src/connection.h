//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef CONNECTION_H
#define CONNECTION_H


#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QSslSocket>

#include <memory>  // for std::unique_ptr

#include "packet.h"

// forward declarations
class TCPThread;
class Packet;

/**
 * @brief RAII-style wrapper for a persistent connection.
 *        Initially minimal; will later own a TCPThread or similar.
 */
class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(const QString &host, quint16 port, const Packet &initialPacket = Packet(), QObject *parent = nullptr);
    ~Connection() override;

    [[nodiscard]] QString id() const;
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isSecure() const;

    void send(const Packet &packet);
    void start();
    void close();

signals:
    // NEW: forward important signals from TCPThread
    void dataReceived(const Packet &packet);
    void stateChanged(const QString &stateMessage);  // or use enum later
    void errorOccurred(const QString &errorString);
    void disconnected();

private slots:
    void onThreadPacketReceived(const Packet &p);
    void onThreadConnectStatus(const QString &msg);
    void onThreadError(QSslSocket::SocketError error);

private:
    void setupThreadConnections();

    QString m_id;
    // QString m_host;       // uncomment later if needed for reconnect
    // quint16 m_port = 0;
    std::unique_ptr<TCPThread> m_thread;  // RAII ownership of the thread
    bool m_threadStarted = false;
    static constexpr int threadShutdownWaitMs = 10000;

protected:
    virtual int threadWaitTimeoutMs() const { return threadShutdownWaitMs; }  // default production value
};


#endif //CONNECTION_H
