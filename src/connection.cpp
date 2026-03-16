//
// Created by Tomas Gallucci on 3/5/26.
//

#include "tcpthread.h"
#include "packet.h"
#include "connection.h"

void Connection::setupThreadConnections()
{
    connect(m_thread.get(), &TCPThread::packetReceived, this, &Connection::onThreadPacketReceived);
    connect(m_thread.get(), &TCPThread::connectStatus, this, &Connection::onThreadConnectStatus);
    connect(m_thread.get(), &TCPThread::error, this, &Connection::onThreadError);
    // Future-proof: if you later add more signals to TCPThread, add connects here
}

// Target constructor
Connection::Connection(std::unique_ptr<TCPThread> thread,
                       bool isIncoming,
                       bool isSecure,
                       bool isPersistent,
                       qintptr socketDescriptor,
                       QObject *parent)
    : QObject(parent),
      m_isIncoming(isIncoming),
      m_isSecure(isSecure),
      m_isPersistent(isPersistent),
      m_socketDescriptor(socketDescriptor)
{
    if (!thread) {
        throw std::invalid_argument("Thread must be provided");
    }

    m_thread = std::move(thread);
    m_thread->setParent(this);

    assignUniqueId();
    setupThreadConnections();
    start();
}

/* Client/outgoing constructor (delegates) */
Connection::Connection(const QString &host,
                    quint16 port,
                    const Packet &initialPacket,
                    QObject *parent,
                    std::unique_ptr<TCPThread> thread)
    : Connection(thread ? std::move(thread)
                        : std::make_unique<TCPThread>(host, port, initialPacket, nullptr),
                 false, false, true, -1, parent)
{
}

// Server/incoming constructor (delegates, preserves member assignments)
Connection::Connection(int socketDescriptor, bool isSecure, bool isPersistent, QObject *parent, std::unique_ptr<TCPThread> thread)
    : Connection(thread ? std::move(thread)
                        : std::make_unique<TCPThread>(socketDescriptor, isSecure, isPersistent, nullptr),
                 true, isSecure, isPersistent, socketDescriptor, parent)
{
}

Connection::~Connection()
{
    // NEW: RAII cleanup – close and wait for thread
    close();

    // Wait with a generous timeout; log if it hangs
    if (m_thread && m_threadStarted && !m_thread->wait(m_threadWaitTimeoutMs)) {
        qWarning() << "In ~Connection(): TCPThread for" << m_id << "did not finish within " << m_threadWaitTimeoutMs << " ms";
    }
    // unique_ptr will delete it automatically here
}

void Connection::close()
{
    if (!m_thread || !m_threadStarted) {
        qDebug() << "close() called but thread not started or already closed";
        return;
    }

    qDebug() << "Connection::close() for" << m_id;

    m_thread->closeConnection();          // sets closeRequest + interruption
    m_thread->requestInterruption();      // extra safety

    // Optional: short wait if you want to ensure quick exit
    // but DON'T block forever here — app might call close() on shutdown
    if (!m_thread->wait(2000)) {
        qWarning() << "Thread for" << m_id << "did not exit within 2 seconds after close request";
    }

    if (m_thread && !m_threadStarted) {
        qDebug() << "close() called but thread not started for" << m_id;
    }

    m_threadStarted = false;
    emit disconnected();

    qDebug() << "close() completed for" << m_id;
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
        qDebug() << "setting m_threadStarted to true inside if is running";
        m_threadStarted = true;
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
