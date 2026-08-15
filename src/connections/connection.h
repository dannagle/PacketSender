//
// Created by Tomas Gallucci on 6/16/26.
//

#ifndef CONNECTION_H
#define CONNECTION_H


#include <QObject>
#include "packet.h"

/*
 * Connection Hierarchy
 *
 * Connection is the abstract base class for all connection types in PacketSender.
 * It provides a protocol-agnostic public API and signal interface.
 *
 * The intended hierarchy is:
 *
 *   Connection (abstract)
 *       │
 *       ├── BaseTcpConnection
 *       │       ├── OutgoingTcpConnection
 *       │       └── IncomingTcpConnection
 *       │
 *       ├── BaseDtlsConnection
 *       │       ├── OutgoingDtlsConnection
 *       │       └── IncomingDtlsConnection
 *       │
 *       ├── BaseUdpConnection
 *       │       └── UdpConnection (usually no separate in/out)
 *       │
 *       └── ... (future protocols: RS-232 [aka serial] QUIC, WebSocket, SCTP, etc.)
 *
 * Design Principle:
 * - Connection itself knows nothing about specific protocols.
 * - Each protocol has a BaseXXXConnection that holds protocol-specific
 *   common behavior and thread ownership.
 * - IncomingXXXConnection and OutgoingXXXConnection provide the final
 *   specialized behavior for each direction.
 */

class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(QObject* parent = nullptr);
    ~Connection() override;

    enum class State {
        Created,
        Active,
        Inactive,
        Idle,
        Closing,
        Closed,
        Error
    };

    [[nodiscard]] State getConnectionState() const {return state_;}
    [[nodiscard]] QString getConnectionStateAsQstring() const;
    [[nodiscard]] static QString stateToString(State state);

    // TCP methods
    virtual void send(const Packet& packet) = 0;
    virtual void receiveData(int socketDescriptor, bool isSecure = false, bool persistent = false) = 0;

    virtual void close() = 0;

    [[nodiscard]] virtual QString id() const;
    [[nodiscard]] virtual bool isConnected() const = 0;
    [[nodiscard]] virtual bool isSecure() const = 0;
    [[nodiscard]] virtual bool isPersistent() const = 0;
    [[nodiscard]] virtual bool isIncoming() const = 0;

    signals:
    void dataReceived(const Packet& packet);
    void packetSent(const Packet& packet);
    void stateChanged(const QString& message);
    void errorOccurred(const QString& errorString);
    void disconnected();

protected:
    void assignUniqueId();
    [[nodiscard]] virtual QString getClassName() const;

    virtual void setupSignalConnections() = 0;

    std::optional<QString> id_;

    std::atomic<State> state_ = State::Created;
};

std::ostream& operator<<(std::ostream& os, const Connection::State& state);

#endif //CONNECTION_H
