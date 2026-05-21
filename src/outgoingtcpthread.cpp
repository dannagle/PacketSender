//
// Created by Tomas Gallucci on 5/9/26.
//

#include "outgoingtcpthread.h"

#include <QMetaEnum>

OutgoingTcpThread::OutgoingTcpThread(QSslSocket* socket, const Packet& packetToSend, QObject* parent)
:BaseTcpThread(socket, parent), sendPacket(packetToSend)
{
    if (packetToSend.toIP.isEmpty()) {
        throw std::invalid_argument("OutgoingTcpThread: packetToSend.toIP cannot be empty");
    }

    const bool destinationPortHasBeenSet = packetToSend.port > 0;
    if (!destinationPortHasBeenSet) {
        throw std::invalid_argument("OutgoingTcpThread: packetToSend.port must be set to a positive integer value");
    }
}

// Convenience constructor
OutgoingTcpThread::OutgoingTcpThread(const Packet& packetToSend,
                                     QObject* parent)
    : OutgoingTcpThread(new QSslSocket(), packetToSend, parent)   // delegates to main constructor
{
}

OutgoingTcpThread::~OutgoingTcpThread()
{
}

QString OutgoingTcpThread::getDestinationAddress() const
{
    return sendPacket.toIP;
}

unsigned int OutgoingTcpThread::getDestinationPort() const
{
    return sendPacket.port;
}

bool OutgoingTcpThread::isValid() const
{
    if (sendPacket.toIP.isEmpty() || sendPacket.port == 0)
    {
        return false;
    }
    return BaseTcpThread::isValid();
}

void OutgoingTcpThread::outgoingConnectionDebugMessage(const bool connectSuccess)
{
    const auto s = getSocket();
    qDebug() << "[OutgoingTcpThread plain connect] ========================================";
    qDebug() << "  waitForConnected() returned:" << connectSuccess;
    qDebug() << "  socket state:" << s->state();
    qDebug() << "  socket error:" << s->errorString();
    qDebug() << "  peer:" << s->peerAddress().toString() << ":" << s->peerPort();
    qDebug() << "  local port:" << s->localPort();
    qDebug() << "================================================================";
}

void OutgoingTcpThread::prepareOutgoingPacket()
{
    sendPacket.fromIP = "You";
    sendPacket.timestamp = QDateTime::currentDateTime();
    sendPacket.name = sendPacket.timestamp.toString(DATETIMEFORMAT);
}

void OutgoingTcpThread::sendOutgoingPacket()
{
    BaseTcpThread::sendOutgoingPacket(sendPacket);
}

void OutgoingTcpThread::closeConnection()
{
    const auto s = getSocket();
    if (s)
    {
        if (getSocketState() == QAbstractSocket::ConnectedState ||
            getSocketState() == QAbstractSocket::ClosingState) {
            QDEBUG() << "got inside if statement that calls disconnectFromHost()";
            s->disconnectFromHost();
            s->waitForDisconnected(500);  // shorter timeout is fine here
        }

        s->close();

        emit connectionStatus("Disconnected");
        QDEBUG() << "Single packet sent. Disconnected.";
    }
}

void OutgoingTcpThread::run()
{
    QDEBUG() << "OutgoingTcpThread::run() started for" << sendPacket.toIP << ":" << sendPacket.port;

    if (sendPacket.isSSL()) {
        qWarning() << "SSL not yet implemented in OutgoingTcpThread";
        return;
    }

    // === Plain TCP - single send (non-persistent for now) ===
    getSocket()->connectToHost(sendPacket.toIP,
                        sendPacket.port,
                        QIODevice::ReadWrite,
                        getIPConnectionProtocol());

    bool connectSuccess = getSocket()->waitForConnected(5000);
    outgoingConnectionDebugMessage(connectSuccess);

    if (connectSuccess) {
        emit connectionStatus("Connected");

        if (sendPacket.delayAfterConnect > 0) {
            QDEBUG() << "sleeping" << sendPacket.delayAfterConnect;
            sleep(1000 * sendPacket.delayAfterConnect);
        }

        // Send the packet once
        prepareOutgoingPacket();
        sendOutgoingPacket();
        closeConnection();
    } else {
        emit connectionStatus("Could not connect.");
        emit errorMessage("Could not connect to " + sendPacket.toIP + ":" + QString::number(sendPacket.port));

        sendPacket.errorString = "Could not connect";
        emit packetSent(sendPacket);
    }
}

bool OutgoingTcpThread::shouldContinuePersistentLoop() const
{
    QDEBUG() << "\nshouldContinuePersistentLoop()\n"
             << "isInterruptionRequested(): " << isInterruptionRequested() << "\n"
             << "getSocket(): " << getSocket() << "\n"
             << "getSocketState(): " << getSocketState() << "\n"
             << "getSocketState() == QAbstractSocket::ConnectedState: " << (getSocketState() == QAbstractSocket::ConnectedState) << "\n"
             << "!shouldStop(): " << !shouldStop() << "\n";

    bool result =  !shouldStop() &&
           getSocket() &&
           getSocketState() == QAbstractSocket::ConnectedState;

    QDEBUG() << "result: " << result;
    return result;
}

bool OutgoingTcpThread::shouldStopPersistentConnectionLoop() const
{
    return shouldStop();
}

void OutgoingTcpThread::handlePersistentIdleCase(int idleThresholdMs)
{
    const QDateTime now = QDateTime::currentDateTime();

    QDEBUG() << "IDLE PATH TAKEN"
             << "hexString empty =" << sendPacket.hexString.isEmpty()
             << "persistent =" << sendPacket.persistent
             << "bytesAvailable =" << (getSocket() ? getSocket()->bytesAvailable() : -1);

    // NOSONAR - if-with-initializer reduces readability here
    if (!lastIdleStatusEmitTime.has_value() ||
        lastIdleStatusEmitTime->msecsTo(now) >= idleThresholdMs) {

        emit connectionStatus("Connected and idle.");
        QDEBUG() << ">>> Emitted 'Connected and idle.'";
        lastIdleStatusEmitTime = now;
        }

    interruptibleWaitForReadyRead(200);
}

Packet OutgoingTcpThread::buildReceivedPacket()
{
    Packet p;
    p.timestamp = QDateTime::currentDateTime();
    p.name = "Received (Persistent)";
    p.tcpOrUdp = sendPacket.tcpOrUdp;

    // Direction reversal
    p.toIP = "You";
    p.fromIP = sendPacket.toIP;
    p.fromPort = getSocket() ? getSocket()->localPort() : 0;

    if (getSocket()) {
        QByteArray data = readSocketData();
        p.hexString = Packet::byteArrayToHex(data);
    }

    return p;
}

void OutgoingTcpThread::processIncomingData()
{
    if (!getSocket() || getSocket()->bytesAvailable() == 0) {
        return;
    }

    QDEBUG() << "past early return";

    Packet received = buildReceivedPacket();
    if (!received.hexString.isEmpty()) {
        emit packetReceived(received);

        // TODO: Smart response logic will go here later
    }
}

void OutgoingTcpThread::idleDebugMessage(bool isIdleCondition) const
{
    QDEBUG() << "Idle condition check:"
        << "hexString.empty() =" << sendPacket.hexString.isEmpty()
        << "persistent =" << sendPacket.persistent
        << "bytesAvailable() =" << getSocket()->bytesAvailable()
        << "→ isIdleCondition =" << isIdleCondition;
}

void OutgoingTcpThread::persistentConnectionLoop()
{
    QDEBUG() << "Entering persistent connection loop for" << sendPacket.toIP << ":" << sendPacket.port;

    if (shouldStopPersistentConnectionLoop()) {
        qDebug() << "Early exit from persistent loop due to close request";
        return;
    }

    while (shouldContinuePersistentLoop()) {
        insidePersistent = true;   // keeping for now, we can remove later if unused

        if (shouldStopPersistentConnectionLoop()) {
            break;
        }

        bool isIdleCondition = sendPacket.hexString.isEmpty() &&
                               sendPacket.persistent &&
                               getSocket()->bytesAvailable() == 0;
        idleDebugMessage(isIdleCondition);

        // === Idle path when there's no data to send ===
        if (isIdleCondition) {
            handlePersistentIdleCase(2000);
        } else {
            processIncomingData();
        }
    }

    QDEBUG() << "Exiting persistent connection loop";
}
