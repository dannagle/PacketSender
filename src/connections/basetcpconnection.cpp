//
// Created by Tomas Gallucci on 6/14/26.
//

#include "basetcpconnection.h"

#include "../basetcpthread.h"
#include <QUuid>
#include <QDebug>

BaseTcpConnection::BaseTcpConnection(std::unique_ptr<BaseTcpThread> thread,
                       QObject* parent)
    : Connection(parent)
    , thread_(std::move(thread))
{
    if (!thread_) {
        throw std::invalid_argument("Connection: thread cannot be null");
    }

    // setupSignalConnections();

    qDebug() << "Connection created:" << id_
             << "(thread type:" << thread_->metaObject()->className() << ")";
}

BaseTcpConnection::~BaseTcpConnection()
{
    // close();   // RAII: ensure clean shutdown
}

bool BaseTcpConnection::isConnected() const
{
    return thread_->isConnected();
}

bool BaseTcpConnection::isSecure() const
{
    return thread_->isSocketEncrypted();
}

bool BaseTcpConnection::isPersistent() const
{
    return thread_->isPersistent();
}

bool BaseTcpConnection::isIncoming() const
{
    return thread_->isIncoming();
}
