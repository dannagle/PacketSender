//
// Created by Tomas Gallucci on 3/6/26.
//

#ifndef CONNECTIONMANAGERTESTS_H
#define CONNECTIONMANAGERTESTS_H


#include <QtTest/QTest>
#include "connectionmanager.h"

// NEW: Test-specific subclass to expose private state for verification
class TestConnectionManager : public ConnectionManager
{
public:
    using ConnectionManager::ConnectionManager;

    // Test accessors (expose private members safely)
    const auto& connections() const { return m_connections; }
    quint64 nextId() const { return m_nextId; }
};

class ConnectionManagerTests : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<TestConnectionManager> manager;

private slots:
    void init();
    void cleanup();

    void testCreateReturnsValidId();
    void testCloseRemovesConnection();
    void testShutdownAllClearsAllConnections();
    void testSignalForwardingIncludesId();
};


#endif //CONNECTIONMANAGERTESTS_H
