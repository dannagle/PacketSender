//
// Created by Tomas Gallucci on 6/14/26.
//

#include "basetcpconnection.h"

#include "../basetcpthread.h"
#include <QUuid>
#include <QDebug>

BaseTcpConnection::BaseTcpConnection(std::unique_ptr<BaseTcpThread> thread,
                       QObject* parent)
    : QObject(parent)
    , thread_(std::move(thread))
{
    if (!thread_) {
        throw std::invalid_argument("Connection: thread cannot be null");
    }

    assignUniqueId();
    // setupSignalConnections();

    qDebug() << "Connection created:" << id_
             << "(thread type:" << thread_->metaObject()->className() << ")";
}

BaseTcpConnection::~BaseTcpConnection()
{
    // close();   // RAII: ensure clean shutdown
}

void BaseTcpConnection::assignUniqueId()
{
    if (id_.has_value())
    {
        throw std::runtime_error("unique id is already set for Connection object");
    }

    id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString BaseTcpConnection::id() const
{
    return id_.has_value()? id_.value() : "";
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
