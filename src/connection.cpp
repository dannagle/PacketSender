//
// Created by Tomas Gallucci on 3/5/26.
//

#include "connection.h"
#include "tcpthread.h"
#include "packet.h"

Connection::Connection(const QString &host, quint16 port, const Packet &initialPacket, QObject *parent)
    : QObject(parent)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
    m_thread = std::make_unique<TCPThread>(host, port, initialPacket, this);

    // Signal forwarding (unchanged)
    connect(m_thread.get(), &TCPThread::packetReceived,
            this, &Connection::onThreadPacketReceived);
    connect(m_thread.get(), &TCPThread::connectStatus,
            this, &Connection::onThreadConnectStatus);
    connect(m_thread.get(), &TCPThread::error,
            this, &Connection::onThreadError);

    start();
}

Connection::~Connection()
{
    // NEW: RAII cleanup – close and wait for thread
    close();

    // Wait with a generous timeout; log if it hangs
    if (m_thread && m_threadStarted && !m_thread->wait(threadWaitTimeoutMs())) {
        qWarning() << "TCPThread for" << m_id << "did not finish within 10 seconds";
    }
    // unique_ptr will delete it automatically here
}

void Connection::close()
{
    if (m_thread && m_threadStarted) {
        m_thread->closeConnection();
        emit disconnected();
    } else {
        qDebug() << "close() called but thread not started for" << m_id;
    }
}

QString Connection::id() const
{
    return m_id;
}

// public API
void Connection::send(const Packet &packet)
{
    if (m_thread) {
        m_thread->sendPersistant(packet);
    }
}

void Connection::start()
{
    if (!m_thread) {
        qWarning() << "No thread to start";
        return;
    }

    if (m_thread->isRunning()) {
        qDebug() << "Thread already running for" << m_id;
        return;
    }

    if (!m_thread->isValid()) {
        qWarning() << "Cannot start - thread invalid for" << m_id;
        // Optional: log why (you already have good logging in isValid())
        emit errorOccurred("Cannot start connection - initialization failed");
        return;
    }

    qDebug() << "Starting TCPThread for connection" << m_id;
    m_thread->start();
    m_threadStarted = true;  // only set if start() was called successfully
}

// simple state queries
bool Connection::isConnected() const
{
    // TODO: you may want to track state internally or ask thread
    return m_thread && m_thread->isRunning();
}

bool Connection::isSecure() const
{
    return m_thread ? m_thread->isSecure : false;
}

// NEW: internal forwarders
void Connection::onThreadPacketReceived(const Packet &p)
{
    emit dataReceived(p);
}

void Connection::onThreadConnectStatus(const QString &msg)
{
    emit stateChanged(msg);
}

void Connection::onThreadError(QSslSocket::SocketError error)
{
    QString errStr = QString("Socket error: %1").arg(error);
    emit errorOccurred(errStr);
    emit disconnected();
}
