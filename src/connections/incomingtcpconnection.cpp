//
// Created by Tomas Gallucci on 6/17/26.
//

#include "incomingtcpconnection.h"

IncomingTcpConnection::IncomingTcpConnection(QObject* parent)
: BaseTcpConnection(parent)
{
}

void IncomingTcpConnection::receiveData(const int socketDescriptor, const bool isSecure, const bool persistent)
{
    // Clean up any previous thread (one-shot finished, or we want a fresh one)
    if (thread_)
    {
        thread_->shutdown();
        thread_.reset();
    }

    try
    {
        // Create fresh thread for this send
        thread_ = makeIncomingTcpThread(socketDescriptor, isSecure, persistent);

        // setupSignalConnections();
        thread_->start();
    }
    catch (const std::exception& e)
    {
        qWarning() << "Failed to create IncomingTcpThread for receiveData:" << e.what();
        thread_.reset();           // stay in clean state
        emit errorOccurred(QString("Failed to create connection: %1").arg(e.what()));
    }
}

std::unique_ptr<IncomingTcpThread> IncomingTcpConnection::makeIncomingTcpThread(int socketDescriptor,  bool isSecure, bool persistent)
{
    return std::make_unique<IncomingTcpThread>(socketDescriptor, isSecure, persistent, this);
}

