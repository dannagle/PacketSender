//
// Created by Tomas Gallucci on 4/25/26.
//

#ifndef BASETCPTHREAD_H
#define BASETCPTHREAD_H

#include <QMutex>
#include <qqueue.h>
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

    QString id() const;

    virtual void shutdown();
    bool isThreadRunning() const;
    QString getThreadStateAsString() const;
    virtual void stop();
    [[nodiscard]] virtual bool shouldStop() const;
    virtual bool interruptibleWaitForReadyRead(int timeoutMs);
    [[nodiscard]] virtual bool isInterruptionRequested() const;

    [[nodiscard]] virtual bool isValid() const;
    [[nodiscard]] virtual bool isConnected() const;
    [[nodiscard]] virtual bool isIncoming() const { return false; }
    [[nodiscard]] virtual bool isPersistent() const { return persistent; }
    [[nodiscard]] bool getShouldUseSSL() const {return shouldUseSSL;}
    [[nodiscard]] PacketSenderQSslSocketInterface* getSocketInterface() const;

    // Common query helpers - public because they are safe and widely useful
    [[nodiscard]] virtual bool isSocketEncrypted() const;
    virtual quint16 getPeerPort() const;
    virtual quint16 getLocalPort() const;
    virtual QAbstractSocket::NetworkLayerProtocol getIPConnectionProtocol() const;
    virtual QString getPeerAddressAsString() const;

    std::unique_ptr<PacketSenderQSslSocketInterface> socketInterface;
    virtual void enqueuePacket(const Packet &packet); // only called from main thread

signals:
    void packetSent(const Packet& packet);
    void packetReceived(const Packet& packet);
    void connectionStatus(const QString& message);
    void error(QSslSocket::SocketError socketError);
    void errorMessage(const QString& msg);
    void disconnected();

protected:
    enum class ThreadState {
        Created,
        Running,
        Stopping,
        Stopped,
        Error
    };

    QMutex sendQueueMutex;
    QQueue<Packet> sendQueue;
    std::atomic<bool> acceptingSends{true};

    virtual void drainSendQueue(); // only called from worker thread inside *TcpThread objects

    std::unique_ptr<PacketSenderQSslSocketInterface>& getSocketPtrByReference() {return socketInterface;}
    [[nodiscard]] virtual QAbstractSocket::SocketState getSocketState() const;
    [[nodiscard]] virtual QByteArray readSocketData();

    virtual bool isValidForSending(Packet& packet, QString* errorMessage) const;
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

    bool closeRequest = false;
    bool shouldUseSSL = false;
    bool persistent = false;

    void assignUniqueId();
    std::optional<QString> id_;
    std::atomic<ThreadState> threadState = ThreadState::Created;

    bool hasSslError = false;

#ifdef CONSOLE_MODE
    bool consoleMode = true;
#else
    bool consoleMode = false;
#endif

};



#endif // BASETCPTHREAD_H
