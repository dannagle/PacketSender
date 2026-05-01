//
// Created by Tomas Gallucci on 4/25/26.
//

#ifndef BASETCPTHREAD_H
#define BASETCPTHREAD_H

#include <QThread>
#include <QSslSocket>
#include "packet.h"

/**
 * Base class for all TCP thread implementations.
 * Contains common socket management, signal wiring, and helper methods.
 */
class BaseTcpThread : public QThread
{
    Q_OBJECT

public:
    /**
     * Takes ownership of the socket.
     * Throws std::invalid_argument if socket is null.
     */
    explicit BaseTcpThread(QSslSocket* socket, QObject* parent = nullptr);
    ~BaseTcpThread() override;

    [[nodiscard]] virtual bool isValid() const;
    [[nodiscard]] QSslSocket* getSocket() const;
    //
    // virtual void closeConnection();
    // virtual void sendPersistent(const Packet& packet);

    // Common query helpers - public because they are safe and widely useful
    [[nodiscard]] virtual bool isSocketEncrypted() const;
    virtual quint16 getPeerPort() const;
    virtual quint16 getLocalPort() const;
    virtual QAbstractSocket::NetworkLayerProtocol getIPConnectionProtocol() const;
    virtual QString getPeerAddressAsString() const;
    //
    // signals:
    //     void packetReceived(const Packet& packet);
    //     void packetSent(const Packet& packet);
    //     void connectStatus(const QString& message);
    //     void error(QSslSocket::SocketError socketError);

protected:
    // virtual void run() override = 0;
    //
    // void wireupSocketSignals(QSslSocket* socket);

    [[nodiscard]] virtual QHostAddress getSocketPeerAddress() const;

    QSslSocket* socket = nullptr;
    bool managedByConnection = false;
    bool closeRequest = false;
    bool insidePersistent = false;

// protected slots:
//     virtual void onConnected();
//     virtual void onSocketError(QSslSocket::SocketError error);
//     virtual void onStateChanged(QAbstractSocket::SocketState state);
};

#endif // BASETCPTHREAD_H
