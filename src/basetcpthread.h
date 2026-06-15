//
// Created by Tomas Gallucci on 4/25/26.
//

#ifndef BASETCPTHREAD_H
#define BASETCPTHREAD_H

#include <QThread>
#include <QSslSocket>
#include "packet.h"
#include "packetsenderqsslsocketinterface.h"

/**
 * Base class for all TCP thread implementations.
 * Contains common socket management, signal wiring, and helper methods.
 */
class BaseTcpThread : public QThread
{
    Q_OBJECT

public:
    void debugSocketState() const;
    /**
     * Takes ownership of the socket.
     * Throws std::invalid_argument if socket is null.
     */
    explicit BaseTcpThread(PacketSenderQSslSocketInterface* socketInterface,
                           QObject* parent = nullptr);
    ~BaseTcpThread() override;

    virtual void stop();
    virtual bool shouldStop() const;
    virtual bool interruptibleWaitForReadyRead(int timeoutMs);

    [[nodiscard]] virtual bool isValid() const;
    [[nodiscard]] virtual bool isConnected() const;
    [[nodiscard]] bool getShouldUseSSL() const {return shouldUseSSL;}
    PacketSenderQSslSocketInterface* getSocketInterface() const;
    // virtual void sendPersistent(const Packet& packet);

    // Common query helpers - public because they are safe and widely useful
    [[nodiscard]] virtual bool isSocketEncrypted() const;
    virtual quint16 getPeerPort() const;
    virtual quint16 getLocalPort() const;
    virtual QAbstractSocket::NetworkLayerProtocol getIPConnectionProtocol() const;
    virtual QString getPeerAddressAsString() const;

    std::unique_ptr<PacketSenderQSslSocketInterface> socketInterface;

    signals:
    void packetSent(const Packet& packet);
    void packetReceived(const Packet& packet);
    void connectionStatus(const QString& message);
    void error(QSslSocket::SocketError socketError);
    void errorMessage(const QString& msg);

protected:
    // virtual void run() override = 0;
    //
    // void wireupSocketSignals(QSslSocket* socket);

    std::unique_ptr<PacketSenderQSslSocketInterface>& getSocketPtrByReference() {return socketInterface;}
    [[nodiscard]] virtual QAbstractSocket::SocketState getSocketState() const;
    [[nodiscard]] virtual QByteArray readSocketData();

    virtual void sendOutgoingPacket(Packet& packet);
    virtual void closeConnection();
    virtual void sleep(unsigned long usec);

    [[nodiscard]] virtual QHostAddress getSocketPeerAddress() const;
    [[nodiscard]] virtual bool isSocketValid() const;

    // ssl cert methods
    void loadSnakeOilCertificate();
    void loadSnakeOilKey();
    virtual void loadSnakeOilCerts();
    virtual void loadSSLCerts(bool allowSnakeOil);

    bool managedByConnection = false;
    bool closeRequest = false;
    bool shouldUseSSL = false;

#ifdef CONSOLE_MODE
    bool consoleMode = true;
#else
    bool consoleMode = false;
#endif


// protected slots:
//     virtual void onConnected();
//     virtual void onSocketError(QSslSocket::SocketError error);
//     virtual void onStateChanged(QAbstractSocket::SocketState state);
};

#endif // BASETCPTHREAD_H
