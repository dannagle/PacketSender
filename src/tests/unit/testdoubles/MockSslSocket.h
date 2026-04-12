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

    [[nodiscard]] QAbstractSocket::SocketState getMockState() const
    {
        qDebug() << "=== MOCK mockState() called → returning" << mockState;
        return mockState;
    }


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
    void setMockConnected(bool val)
    {
        mockConnected = val;

        if (mockConnected)
        {
            mockState = QAbstractSocket::ConnectedState;
        } else
        {
            mockState = QAbstractSocket::UnconnectedState;
        }
    }

    void setMockBytesAvailable(qint64 bytes) { mockBytesAvailable = bytes; }

    qint64 getMockBytesAvailable() const
    {
        qDebug() << "=== MOCK getMockBytesAvailable() called → returning" << mockBytesAvailable;
        return mockBytesAvailable;
    }

    void setMockEncrypted(bool val) { mockEncrypted = val; }
    void setMockSslErrors(const QList<QSslError> &errors) { mockSslErrors = errors; }

private:
    bool mockConnected = false;
    bool mockEncrypted = false;
    QList<QSslError> mockSslErrors;
    QSslCipher mockCipher;
    QAbstractSocket::SocketState mockState = QAbstractSocket::UnconnectedState;
    qint64 mockBytesAvailable = 0;
};
#endif //MOCKSSLSOCKET_H
