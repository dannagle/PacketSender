//
// Created by Tomas Gallucci on 3/17/26.
//

#ifndef MOCKSSLSOCKET_H
#define MOCKSSLSOCKET_H

#include <QSslCipher>
#include <QObject>
#include <QSslSocket>

// Mock QSslSocket for testing (or use a real one in a controlled way)
class MockSslSocket : public QSslSocket {
    Q_OBJECT
public:
    explicit MockSslSocket(QObject *parent = nullptr)
        // Do NOT call QSslSocket(parent) here
        // Qt handles initialization internally via d-pointer
    {
        // Your init code if any
        setParent(parent);  // optional, but good practice
    }

    // FIX: Explicitly delete copy/move to match base class
    MockSslSocket(const MockSslSocket &) = delete;
    MockSslSocket& operator=(const MockSslSocket &) = delete;
    MockSslSocket(MockSslSocket &&) = delete;
    MockSslSocket& operator=(MockSslSocket &&) = delete;


    void setMockCipher(const QSslCipher &cipher) { mockCipher = cipher; }

    QSslCipher sessionCipher() const { return mockCipher; }

    bool waitForConnected(int msecs = 30000) override { qDebug() << "=== MOCK waitForConnected called → returning" << mockConnected; return mockConnected; }
    bool waitForEncrypted(int msecs = 30000) { qDebug() << "=== MOCK waitForEncrypted called → returning" << mockEncrypted; return mockEncrypted; }
    bool isEncrypted() const {qDebug() << "=== MOCK isEncrypted called → returning" << mockEncrypted; return mockEncrypted; }

    QList<QSslError> sslErrors() const { return mockSslErrors; }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QList<QSslError> sslHandshakeErrors() const { return mockSslErrors; }
#endif

    // Mock setters
    void setMockConnected(bool val) { mockConnected = val; }
    void setMockEncrypted(bool val) { mockEncrypted = val; }
    void setMockSslErrors(const QList<QSslError> &errors) { mockSslErrors = errors; }

private:
    bool mockConnected = false;
    bool mockEncrypted = false;
    QList<QSslError> mockSslErrors;
    QSslCipher mockCipher;
};
#endif //MOCKSSLSOCKET_H
