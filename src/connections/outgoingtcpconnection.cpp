//
// Created by Tomas Gallucci on 6/18/26.
//

#include "outgoingtcpconnection.h"

OutgoingTcpConnection::OutgoingTcpConnection(QObject* parent)
    : BaseTcpConnection(parent)
{
}

void OutgoingTcpConnection::send(const Packet& packet)
{
    QString errorMessage;
    if (!packet.isValidForSending(&errorMessage))
    {
        QDEBUG() << "from OutgoingTcpConnection::send: " << errorMessage;
        return;
    }

    const bool canReuseThread =
        thread_
        && thread_->isThreadRunning()
        && isPersistent();

    qDebug() << "Outgoing send"
         << "reuse=" << canReuseThread
         << "localPort=" << (thread_ ? thread_->getLocalPort() : 0)
         << "thread=" << thread_.get();

    if (canReuseThread)
    {
        // Live persistent connection — enqueue only, do not touch thread_
        thread_->enqueuePacket(packet);
        return;
    }

    // Clean up any previous thread (e.g. one-shot finished, or, in future, if we want a fresh one)
    if (thread_)
    {
        thread_->shutdown();
        thread_.reset();
    }
    try
    {
        // Create fresh thread for this send
        thread_ = std::move(makeOutgoingTcpThread(packet));
        setupSignalConnections();
        moveSocketToWorkerThread();
        thread_->start();
    }
    catch (const std::exception& e)
    {
        qWarning() << "Failed to create OutgoingTcpThread for send:" << e.what();
        thread_.reset();           // stay in clean state
        emit errorOccurred(QString("Failed to create connection: %1").arg(e.what()));
    }
}

std::unique_ptr<OutgoingTcpThread> OutgoingTcpConnection::makeOutgoingTcpThread(const Packet& packet)
{
    return std::make_unique<OutgoingTcpThread>(packet, this);
}
