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
#include "tcpthread.h"

// forward declarations
class Packet;

/**
 * @brief RAII-style wrapper for a persistent connection.
 *        Initially minimal; will later own a TCPThread or similar.
 */
class Connection : public QObject
{
    Q_OBJECT

public:
    // target constructor
    explicit Connection(std::unique_ptr<TCPThread> thread,
                        bool isIncoming = false,
                        bool isSecure = false,
                        bool isPersistent = true,
                        qintptr socketDescriptor = -1,
                        QObject *parent = nullptr);
    explicit Connection(const QString &host,
                        quint16 port,
                        const Packet &initialPacket = Packet(),
                        QObject *parent = nullptr,
                        std::unique_ptr<TCPThread> thread = nullptr);
    // Server/incoming constructor
    explicit Connection(int socketDescriptor,
                        bool isSecure = false,
                        bool isPersistent = true,
                        QObject *parent = nullptr,
                        std::unique_ptr<TCPThread> thread = nullptr);
    ~Connection() override;

    [[nodiscard]] QString id() const;
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isSecure() const;
    [[nodiscard]] bool isIncoming() const { return m_isIncoming; }
    [[nodiscard]] bool isPersistent() const { return m_isPersistent; }  // rename if you prefer isPersistentConnection()
    [[nodiscard]] qintptr socketDescriptor() const { return m_socketDescriptor; }

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
    void shutdownThreadSafely(int timeoutMs = 2000);
    bool m_isClosing = false;



    QString m_id;
    // QString m_host;       // uncomment later if needed for reconnect
    // quint16 m_port = 0;
    std::unique_ptr<TCPThread> m_thread;  // RAII ownership of the thread
    bool m_threadStarted = false;
    bool m_isIncoming = false;
    bool m_isSecure = false;
    bool m_isPersistent = false;
    qintptr m_socketDescriptor = -1;

    static constexpr int threadShutdownWaitMs = 10000;

protected:
    [[nodiscard]] TCPThread* getThread() const { return m_thread.get(); }
    [[nodiscard]] bool getThreadStarted() const { return m_threadStarted; }

    int m_threadWaitTimeoutMs = 10000;

    void assignUniqueId() {m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);}
};


#endif //CONNECTION_H
