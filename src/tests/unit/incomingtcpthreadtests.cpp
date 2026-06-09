//
// Created by Tomas Gallucci on 6/8/26.
//

#include <QtTest>

#include "incomingtcpthreadtests.h"

#include <QTcpServer>

#include "../../incomingtcpthread.h"

QTcpServer& startQTcpServer()
{
    auto *server = new QTcpServer();
    server->listen(QHostAddress::LocalHost);
    return *server;
}

int getValidDescriptor()
{
    const int validDescriptor = static_cast<int>(startQTcpServer().socketDescriptor());
    QDEBUG() << "validDescriptor: " << validDescriptor;
    return validDescriptor;
}

void IncomingTcpThreadTests::testConstructor_assignsSocketDescriptor()
{
    const int validDescriptor = getValidDescriptor();

    auto const thread = IncomingTcpThread(validDescriptor);
    QCOMPARE(thread.getSocketDescriptor(), validDescriptor);
}

void IncomingTcpThreadTests::testConstructor_assignsIsSecure()
{
    auto const thread = IncomingTcpThread(getValidDescriptor(), true);
    QCOMPARE(thread.getShouldUseSSL(), true);
}


