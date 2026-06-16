//
// Created by Tomas Gallucci on 6/14/26.
//

#include "connection.h"

#include "basetcpthread.h"
#include <QUuid>
#include <QDebug>

Connection::Connection(std::unique_ptr<BaseTcpThread> thread,
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

Connection::~Connection()
{
    // close();   // RAII: ensure clean shutdown
}

void Connection::assignUniqueId()
{
    if (id_.has_value())
    {
        throw std::runtime_error("unique id is already set for Connection object");
    }

    id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Connection::id() const
{
    return id_.has_value()? id_.value() : "";
}

bool Connection::isConnected() const
{
    return thread_->isConnected();
}

bool Connection::isSecure() const
{
    return thread_->isSocketEncrypted();
}

bool Connection::isPersistent() const
{
    return thread_->isPersistent();
}

bool Connection::isIncoming() const
{
    return thread_->isIncoming();
}
