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

BaseTcpConnection::~BaseTcpConnection()
{
    if (thread_) {
        thread_->stop(); // RAII: ensure clean shutdown
        thread_->quit();
        thread_->wait();        // Important: wait for the thread to finish
    }
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
}
