//
// Created by Tomas Gallucci on 6/16/26.
//

#include "connection.h"

#include <QUuid>

Connection::Connection(QObject* parent)
{
    this->setParent(parent);
    assignUniqueId();
}

Connection::~Connection()
{
    QDEBUG() << "Connection::~Connection() state_:" << getConnectionStateAsQstring();
}

QString Connection::getConnectionStateAsQstring() const
{
    switch (state_.load())
    {
        case State::Created: return "Created";
        case State::Active: return "Active";
        case State::Inactive: return "Inactive";
        case State::Idle: return "Idle";
        case State::Closing: return "Closing";
        case State::Closed: return "Closed";
        case State::Error: return "Error";
    }
}

QString Connection::id() const
{
    return id_.has_value() ? id_.value() : QString();
}

void Connection::assignUniqueId()
{
    if (id_.has_value())
    {
        throw std::runtime_error("unique id is already set for Connection object");
    }

    id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Connection::getClassName() const
{
    return metaObject()->className();
}
