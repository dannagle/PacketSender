//
// Created by Tomas Gallucci on 6/14/26.
//

#include "basetcpconnection.h"

#include "../basetcpthread.h"
#include <QUuid>
#include <QDebug>
#include<exception>

BaseTcpConnection::BaseTcpConnection(QObject* parent)
    : Connection(parent)
{
}

bool BaseTcpConnection::isConnected() const
{
    return thread_ && thread_->isConnected();
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

void BaseTcpConnection::send(const Packet& packet)
{
    const auto errorMessage = "Unsupported Operation: "
        + getClassName() + " cannot send Packet";
    throw std::runtime_error(errorMessage.toUtf8());
}

void BaseTcpConnection::receiveData(const int socketDescriptor, const bool isSecure, const bool persistent)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(isSecure);
    Q_UNUSED(persistent);

    const auto errorMessage = "Unsupported Operation: "
        + getClassName() + " cannot receive data";
    throw std::runtime_error(errorMessage.toUtf8());
}

void BaseTcpConnection::terminateConnection()
{
    qDebug() << qPrintable("in BaseTcpConnection::terminateConnection");

    const bool connectedBeforeCloseCalled = isConnected();

    if (thread_)
    {
        thread_->shutdown();
        thread_.reset();
    }

    if (connectedBeforeCloseCalled && !isConnected())
    {
        emit disconnected();
    }
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

