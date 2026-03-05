//
// Created by Tomas Gallucci on 3/5/26.
//

#include "connection.h"

Connection::Connection(const QString &host, quint16 port, QObject *parent)
    : QObject(parent)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
    // In future steps:
    // m_thread = new TCPThread(host, port, this);
    // connect signals/slots as needed
    // m_thread->start();
}

Connection::~Connection()
{
    // In future steps:
    // if (m_thread) {
    //     m_thread->stop();
    //     m_thread->wait(5000);
    //     delete m_thread;  // or let unique_ptr handle it
    // }
}
QString Connection::id() const
{
    return m_id;
}
