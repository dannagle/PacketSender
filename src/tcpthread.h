/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */
#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QThread>
#include <QSslSocket>
#include "packet.h"

class TCPThread : public QThread
{
        Q_OBJECT

    public:
        TCPThread(int socketDescriptor, QObject *parent);
        TCPThread(Packet sendPacket, QObject *parent);

        // NEW constructor for Connection-managed persistent client
        TCPThread(const QString &host, quint16 port,
            const Packet &initialPacket = Packet(),
            QObject *parent = nullptr);

        // NEW constructor for Connection-managed incoming/server connections
        TCPThread(int socketDescriptor, bool isSecure, bool isPersistent, QObject *parent = nullptr);
        ~TCPThread() override;

        void sendAnother(Packet sendPacket);
        static void loadSSLCerts(QSslSocket *sock, bool allowSnakeOil);

        void forceShutdown();

        void run();
        bool sendFlag;
        bool incomingPersistent;
        bool closeRequest = false;
        bool isSecure;
        bool isEncrypted();
        Packet packetReply;
        bool consoleMode;
        [[nodiscard]] bool isValid() const;

    signals:
        void error(QSslSocket::SocketError socketError);

        void packetReceived(Packet sendpacket);
        void toStatusBar(const QString & message, int timeout = 0, bool override = false);
        void packetSent(Packet sendpacket);
        void connectStatus(QString message);

    public slots:
        void sendPersistant(Packet sendpacket);

        void closeConnection();
    private slots:
        void wasdisconnected();

        void onConnected();
        void onSocketError(QAbstractSocket::SocketError socketError);
        void onStateChanged(QAbstractSocket::SocketState state);

    private:
        int socketDescriptor;
        QString text;
        void init();
        void writeResponse(QSslSocket *sock, Packet tcpPacket);
        bool insidePersistent;

        void persistentConnectionLoop();

        QString host;
        quint16 port = 0;

    protected:
        // Allow injecting a pre-created socket (primarily for unit testing)
        explicit TCPThread(QSslSocket *preCreatedSocket,
                           const QString &host,
                           quint16 port,
                           const Packet &initialPacket = Packet(),
                           QObject *parent = nullptr);

        QSslSocket* clientSocket();  // non-const (for creation/modification)
        const QSslSocket* clientSocket() const;  // const (for read-only access)

        bool interruptibleWaitForReadyRead(int timeoutMs);

        // Protected accessors — added for unit tests
        [[nodiscard]] QSslSocket* getClientConnection() const { return clientConnection; }
        [[nodiscard]] int getSocketDescriptor() const { return socketDescriptor; }
        [[nodiscard]] bool getIsSecure() const { return isSecure; }
        [[nodiscard]] bool getIncomingPersistent() const { return incomingPersistent; }
        [[nodiscard]] const QString& getHost() const { return host; }
        [[nodiscard]] quint16 getPort() const { return port; }
        [[nodiscard]] bool getSendFlag() const { return sendFlag; }
        [[nodiscard]] bool getManagedByConnection() const { return m_managedByConnection; }

        [[nodiscard]] virtual bool divideWaitBy10ForUnitTest() const { return false; }
        [[nodiscard]] virtual bool isSocketEncrypted(const QSslSocket &sock) const { return sock.isEncrypted(); };

        virtual QList<QSslError> getSslErrors(QSslSocket *sock) const;
        virtual QList<QSslError> getSslHandshakeErrors(QSslSocket *sock) const;

        Packet sendPacket;
        QAbstractSocket::NetworkLayerProtocol getIPConnectionProtocol() const;
        bool tryConnectEncrypted();
        void wireupSocketSignals();

        QSslSocket * clientConnection;

        // Default implementation uses real socket
        virtual bool checkConnectionAndEncryption();

        virtual std::pair<bool, bool> performEncryptedHandshake();

        // The common logic — can be called from base or test override
        void handleOutgoingSSLHandshake(bool handshakeSucceeded, bool isEncryptedResult);

        // Virtual method for binding — override in test doubles to skip or control
        virtual bool bindClientSocket();

        int destructorWaitMs = 5000;
        bool m_managedByConnection = false;  // flag to skip deleteLater() in run()

        // doesn't need to be public, but we do want to spy on it in unit tests
        virtual void runOutgoingClient();
        virtual void handleIncomingSSLHandshake(QSslSocket& sock);

        virtual Packet buildInitialReceivedPacket(QSslSocket &sock);

};

#endif // TCPTHREAD_H
