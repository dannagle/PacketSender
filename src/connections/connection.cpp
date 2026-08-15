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
    return stateToString(state_);
}

QString Connection::stateToString(const State state)
{
    switch (state)
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

std::ostream& operator<<(std::ostream& os, const Connection::State& state)
{
    switch (state)
    {
        case Connection::State::Created: os << "Created"; return os;
        case Connection::State::Active: os << "Active"; return os;
        case Connection::State::Inactive: os << "InActive"; return os;
        case Connection::State::Idle: os << "Idle"; return os;
        case Connection::State::Closing: os << "Closing"; return os;
        case Connection::State::Closed: os << "Closed"; return os;
        case Connection::State::Error: os << "Error"; return os;
    }
}
