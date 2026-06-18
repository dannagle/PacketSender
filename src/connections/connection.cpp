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
