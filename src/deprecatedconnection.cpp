//
// Created by Tomas Gallucci on 3/5/26.
//

#include "tcpthread.h"
#include "packet.h"
#include "deprecatedconnection.h"

void DeprecatedConnection::setupThreadConnections()
{
    connect(m_thread.get(), &TCPThread::packetReceived, this, &DeprecatedConnection::onThreadPacketReceived);
    connect(m_thread.get(), &TCPThread::connectStatus, this, &DeprecatedConnection::onThreadConnectStatus);
    connect(m_thread.get(), &TCPThread::error, this, &DeprecatedConnection::onThreadError);
    // Future-proof: if you later add more signals to TCPThread, add connects here
}

// Target constructor
DeprecatedConnection::DeprecatedConnection(std::unique_ptr<TCPThread> thread,
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
DeprecatedConnection::DeprecatedConnection(const QString &host,
                    quint16 port,
                    const Packet &initialPacket,
                    QObject *parent,
                    std::unique_ptr<TCPThread> thread)
    : DeprecatedConnection(thread ? std::move(thread)
                        : std::make_unique<TCPThread>(host, port, initialPacket, nullptr),
                 false, false, true, -1, parent)
{
}

// Server/incoming constructor (delegates, preserves member assignments)
DeprecatedConnection::DeprecatedConnection(int socketDescriptor, bool isSecure, bool isPersistent, QObject *parent, std::unique_ptr<TCPThread> thread)
    : DeprecatedConnection(thread ? std::move(thread)
                        : std::make_unique<TCPThread>(socketDescriptor, isSecure, isPersistent, nullptr),
                 true, isSecure, isPersistent, socketDescriptor, parent)
{
}

DeprecatedConnection::~DeprecatedConnection()
{
    // NEW: RAII cleanup – close and wait for thread
    if (m_thread && m_thread->isRunning()) {
        qWarning() << "~Connection(): thread still running for" << m_id
                   << "— forcing quick shutdown (user did not call close())";

        shutdownThreadSafely(1000);  // shorter timeout in destructor
    }
    // unique_ptr will delete it automatically here
}

void DeprecatedConnection::close()
{
    if (!m_thread || !m_threadStarted) {
        qDebug() << "close() called but thread not started or already closed";
        return;
    }

    if (m_isClosing) {
        qDebug() << "close() already in progress for" << m_id;
        return;
    }

    m_isClosing = true;

    qDebug() << "Connection::close() for" << m_id;

    shutdownThreadSafely(2000);           // normal graceful timeout


    m_threadStarted = false;
    m_isClosing = false;
    emit disconnected();

    qDebug() << "close() completed for" << m_id;
}

void DeprecatedConnection::shutdownThreadSafely(int timeoutMs)
{
    if (!m_thread || !m_threadStarted) {
        qDebug() << "shutdownThreadSafely: no active thread for" << m_id;
        return;
    }

    qDebug() << "shutdownThreadSafely: requesting thread stop for" << m_id
             << "(timeout:" << timeoutMs << "ms)";

    m_thread->closeConnection();          // sets closeRequest + interruption
    m_thread->requestInterruption();

    bool exitedCleanly = m_thread->wait(timeoutMs);

    if (!exitedCleanly) {
        qWarning() << "shutdownThreadSafely: thread did not exit within" << timeoutMs << "ms";
        m_thread->forceShutdown();        // abort socket to unblock waits

        // One last short chance
        if (!m_thread->wait(1000)) {
            qWarning() << "shutdownThreadSafely: force failed — terminating thread";
            m_thread->terminate();        // absolute last resort
        }
    }

    qDebug() << "shutdownThreadSafely: completed for" << m_id
             << "(exited cleanly:" << exitedCleanly << ")";
}

QString DeprecatedConnection::id() const
{
    return m_id;
}

// public API
void DeprecatedConnection::send(const Packet &packet)
{
    if (m_thread) {
        m_thread->sendPersistant(packet);
    }
}

void DeprecatedConnection::start()
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
bool DeprecatedConnection::isConnected() const
{
    // TODO: you may want to track state internally or ask thread
    return m_thread && m_thread->isRunning();
}

bool DeprecatedConnection::isSecure() const
{
    return m_thread ? m_thread->isSecure : false;
}

// NEW: internal forwarders
void DeprecatedConnection::onThreadPacketReceived(const Packet &p)
{
    emit dataReceived(p);
}

void DeprecatedConnection::onThreadConnectStatus(const QString &msg)
{
    emit stateChanged(msg);
}

void DeprecatedConnection::onThreadError(QSslSocket::SocketError error)
{
    QString errStr = QString("Socket error: %1").arg(error);
    emit errorOccurred(errStr);
    emit disconnected();
}
