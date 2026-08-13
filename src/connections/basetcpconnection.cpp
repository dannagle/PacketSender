//
// Created by Tomas Gallucci on 6/14/26.
//

#include "basetcpconnection.h"
#include "../tcpThreads/basetcpthread.h"
#include <QUuid>
#include <QDebug>
#include<exception>

#include "ConnectionStatusMessages.h"

const std::unordered_map<QString, Connection::State> messageToState = {
    {ConnectionStatusMessages::CONNECTED(), Connection::State::Active},
    {ConnectionStatusMessages::COULD_NOT_CONNECT(), Connection::State::Inactive},
    {ConnectionStatusMessages::INCOMING_CONNECTION_ACCEPTED(), Connection::State::Active},
    {ConnectionStatusMessages::ERROR_SOCKET_NOT_CONNECTED(), Connection::State::Inactive},
    {ConnectionStatusMessages::SSL_CONNECTED(), Connection::State::Active},
    {ConnectionStatusMessages::SSL_HANDSHAKE_FAILED(), Connection::State::Inactive},
    {ConnectionStatusMessages::DISCONNECTED(), Connection::State::Inactive},
    {ConnectionStatusMessages::CONNECTED_AND_IDLE(), Connection::State::Active},
    // {ConnectionStatusMessages::ERROR(), Connection::State::Error},
    // {ConnectionStatusMessages::CONNECTING(), Connection::State::Active},
};

Connection::State BaseTcpConnection::stateFromMessage(const QString& msg) const
{
    // qDebug() << "BaseTcpConnection::stateFromMessage msg: " << msg;
    if (auto it = messageToState.find(msg); it != messageToState.end()) {
        // qDebug() << "found " << msg;
        return it->second;
    }
    qDebug() << "BaseTcpConnection::stateFromMessage did not find " << msg;
    return State::Error;   // default
}

void BaseTcpConnection::setState(State state)
{
    state_ = state;
}

BaseTcpConnection::BaseTcpConnection(QObject* parent)
    : Connection(parent)
{
}

BaseTcpConnection::~BaseTcpConnection()
{
    // qDebug() << "BaseTcpConnection destructor started - mutex locked:" << !mutex.tryLock();
    if (state_.load() != State::Closed)
    {
        QDEBUG() << "State: " << getConnectionStateAsQstring();
        terminateConnection();
    }
    QDEBUG() << "state_ in BaseTcpConnection::destructor right before closing brace: " << getConnectionStateAsQstring();
    qDebug() << "BaseTcpConnection destructor finished";
}

bool BaseTcpConnection::isConnected() const
{
    QDEBUG() << "state: " << getConnectionStateAsQstring();
    return state_.load() == State::Active || state_.load() == State::Closing;
}

bool BaseTcpConnection::isSecure() const
{
    return thread_ && thread_->isSocketEncrypted();
}

bool BaseTcpConnection::isPersistent() const
{
    return thread_ && thread_->isPersistent();
}

bool BaseTcpConnection::isIncoming() const
{
    return thread_ && thread_->isIncoming();
}

void BaseTcpConnection::moveSocketToWorkerThread()
{
    if (!thread_) return;
    if (auto *iface = thread_->getSocketInterface()) {
        if (auto *real = iface->rawSocket()) {
            real->setParent(nullptr);
            real->moveToThread(thread_.get());
        }
    }
}

void BaseTcpConnection::send(const Packet& packet)
{
    const auto errorMessage = "Unsupported Operation: "
        + getClassName() + " cannot send Packet";
    throw std::runtime_error(errorMessage.toUtf8());
}

void BaseTcpConnection::receiveData(const int socketDescriptor, const bool isSecure, const bool persistent)
{
    Q_UNUSED(socketDescriptor)
    Q_UNUSED(isSecure)
    Q_UNUSED(persistent)

    const auto errorMessage = "Unsupported Operation: "
        + getClassName() + " cannot receive data";
    throw std::runtime_error(errorMessage.toUtf8());
}

void BaseTcpConnection::terminateConnection()
{
    QDEBUG() << "state in BaseTcpConnection::terminateConnection: " << getConnectionStateAsQstring();

    if (state_.load() == State::Closing || state_.load() == State::Closed)
    {
        return;
    }

    state_ = State::Closing;

    if (!thread_)
    {
        QDEBUG() << "BaseTcpConnection::terminateConnection() thread_ was nullptr";
        return;
    }
    qDebug() << qPrintable("in BaseTcpConnection::terminateConnection past if statements");

    const bool connectedBeforeCloseCalled = isConnected();

    if (thread_)
    {
        thread_->shutdown();
    }

    state_ = State::Closed;

    if (connectedBeforeCloseCalled && !isConnected())
    {
        emit disconnected();
    }

    qDebug() << "BaseTcpConnection destructor finished. state_ is: " << getConnectionStateAsQstring();
}

void BaseTcpConnection::setupSignalConnections()
{
    QDEBUG() << "does thread_ exist?: " << !(!thread_);
    if (!thread_) return;

    qDebug() << "setupSignalConnections() called for thread" << thread_->id();

    connect(thread_.get(), &BaseTcpThread::packetReceived,
            this, &Connection::dataReceived, Qt::DirectConnection);

    connect(thread_.get(), &BaseTcpThread::packetSent,
            this, &Connection::packetSent, Qt::DirectConnection);

    connect(thread_.get(), &BaseTcpThread::connectionStatus,
        this, [this](const QString& msg) {
            // qDebug() << "connectionStatus lambda executed with msg: " << msg;

            if (msg.startsWith("Sending data: ") && state_.load() == State::Active) return;

            setState(stateFromMessage(msg));
            // QDEBUG() << "state_ is now: " << getConnectionStateAsQstring();

            emit stateChanged(msg);
        }, Qt::DirectConnection);

    connect(thread_.get(), &BaseTcpThread::errorMessage,
            this, &Connection::errorOccurred, Qt::DirectConnection);

    connect(thread_.get(), &BaseTcpThread::disconnected,
            this, &Connection::disconnected, Qt::DirectConnection);
}

void BaseTcpConnection::close()
{
    /*
     * close() is the public API. terminate() is the source of truth
     *
     * I don't forsee a reason to do anything else besides (English language) terminate
     * inside of close(). Since I want the same behavior in close() and ~BaseTcpConnection()
     * I want to have that common logic in one place so if the implementation of the public
     * API needs to change in the future, it can do so without having to refactor the common
     * logic. Or, if for some unknown reason the common logic needs to change, it's at least
     * separate from the public API.
     */
    terminateConnection();
}

