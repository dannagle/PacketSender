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
    qDebug() << "OutgoingTcpConnection::send() thread_ ? true: false: " << (thread_ ? true : false);
    // qDebug() << "OutgoingTcpConnection::send() thread_->metaObject()->className(): " << thread_->metaObject()->className();
    qDebug() << "OutgoingTcpConnection::send() thread_.get(): " << thread_.get();
    qDebug() << "OutgoingTcpConnection::send() thread_.currentThreadId(): " << thread_->currentThreadId();
    // Clean up any previous thread (one-shot finished, or we want a fresh one)
    if (thread_)
    {
        qDebug() << "worst debug message ever: got inside if";
        thread_->shutdown();
        thread_.reset();
    }

    try
    {
        qDebug() << "OutgoingTcpConnection::send() got inside the try";
        // Create fresh thread for this send
        thread_ = makeOutgoingTcpThread(packet);

        // setupSignalConnections();
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
