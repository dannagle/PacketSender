//
// Created by Tomas Gallucci on 4/11/26.
//



#include "tcpthread.h"

// HELPERS
QAbstractSocket::SocketState TCPThread::socketState() const
{
    return clientSocket() ? clientSocket()->state() : QAbstractSocket::UnconnectedState;
}

bool TCPThread::shouldContinuePersistentLoop() const
{
    QDEBUG() << "inside shouldContinuePersistentLoop in TcpThread, !isInterruptionRequested: " << !isInterruptionRequested()
            <<  "\n !closeRequest: " << !closeRequest
            <<  "\n clientSocket(): " << clientSocket()
            <<  "\n socketState(): " << socketState()
            <<"\n clientSocket() && socketState() == QAbstractSocket::ConnectedState: " << (clientSocket() && socketState() == QAbstractSocket::ConnectedState);
    return !isInterruptionRequested() &&
           !closeRequest &&
           clientSocket() && socketState() == QAbstractSocket::ConnectedState;
}

qint64 TCPThread::socketBytesAvailable() const
{
    if (clientSocket()) {
        return clientSocket()->bytesAvailable();
    }
    return 0;
}

void TCPThread::deleteSocketLater()
{
    if (clientConnection) {
        clientConnection->deleteLater();
    }
}

QHostAddress TCPThread::getPeerAddress() const
{
    if (clientSocket()) {
        return clientSocket()->peerAddress();
    }
    return QHostAddress();
}

QByteArray TCPThread::readSocketData()
{
    if (clientSocket()) {
        return clientSocket()->readAll();
    }
    return QByteArray();
}

// EXTRACTED FROM TcpThread
void TCPThread::prepareForPersistentLoop(const Packet &initialPacket)
{
    // Socket setup - only for real incoming connections
    if (socketDescriptor > 0) {
        clientConnection = new QSslSocket(this);

        if (!clientSocket()->setSocketDescriptor(socketDescriptor)) {
            qWarning() << "Failed to set socket descriptor on clientConnection for persistent incoming connection";
            delete clientConnection;
            clientConnection = nullptr;
            return;
        }

        QDEBUG() << "Persistent incoming mode entered - using heap clientConnection";
    }

    // Core packet preparation
    sendPacket = initialPacket;
    sendPacket.persistent = true;
    sendPacket.hexString.clear();

    // Set port information from live socket if available
    if (clientSocket() && clientSocket()->state() == QAbstractSocket::ConnectedState) {
        sendPacket.port = clientSocket()->peerPort();
        sendPacket.fromPort = clientSocket()->localPort();   // this is the important one
    }
    // else: leave fromPort and port as they were in initialPacket
    // (unit tests can set them explicitly if they care)
}

void TCPThread::cleanupAfterPersistentConnectionLoop()
{
    qDebug() << "persistentConnectionLoop exiting - cleaning up socket";
    qDebug() << "cleanupAfterPersistentConnectionLoop() called with clientConnection =" << clientConnection;

    if (clientConnection) {
        if (clientSocket()->state() == QAbstractSocket::ConnectedState ||
            clientSocket()->state() == QAbstractSocket::ClosingState) {
            clientSocket()->disconnectFromHost();
            clientSocket()->waitForDisconnected(500);  // shorter timeout is fine here
        }

        clientSocket()->close();

        if (!m_managedByConnection) {
            deleteSocketLater();
        }
        clientConnection = nullptr;  // clear pointer
    }

    emit connectStatus("Disconnected");
}

void TCPThread::handlePersistentIdleCase()
{
    QDEBUG() << "IDLE PATH TAKEN"
                     << " hexString empty =" << sendPacket.hexString.isEmpty()
                     << " persistent =" << sendPacket.persistent
                     << " bytesAvailable =" << clientSocket()->bytesAvailable();

    const QDateTime now = QDateTime::currentDateTime();

    // NOSONAR - if-with-initializer reduces readability here
    if (!lastIdleStatusEmitTime.has_value() || lastIdleStatusEmitTime->msecsTo(now) >= 2000) {
            emit connectStatus("Connected and idle.");
            lastIdleStatusEmitTime = now;
    }

    interruptibleWaitForReadyRead(200);
}

QString TCPThread::getPeerAddressAsString() const
{
    qDebug() << "getPeerAddressAsString() called";
    qDebug() << "  clientSocket() =" << clientSocket();

    if (!clientSocket()) {
        qDebug() << "  → No clientSocket, returning empty string";
        return "";
    }

    QAbstractSocket::NetworkLayerProtocol protocol = getIPConnectionProtocol();
    qDebug() << "  IP protocol =" << protocol;

    if (protocol == QAbstractSocket::IPv6Protocol) {
        QString result = Packet::removeIPv6Mapping(getPeerAddress());
        qDebug() << "  IPv6 result =" << result;
        return result;
    } else {
        QString result = getPeerAddress().toString();
        qDebug() << "  IPv4 result =" << result;
        return result;
    }
}

void TCPThread::sendCurrentPacket()
{
    if(sendPacket.getByteArray().size() > 0) {
        emit connectStatus("Sending data:" + sendPacket.asciiString());
        QDEBUG() << "Attempting write data";
        clientSocket()->write(sendPacket.getByteArray());
        emit packetSent(sendPacket);
    }
}

void TCPThread::handleReceiveBeforeSend()
{
    QDEBUG() << "Wait for data before sending...";
    emit connectStatus("Waiting for data");
    interruptibleWaitForReadyRead(500);

    Packet tcpRCVPacket;
    tcpRCVPacket.hexString = Packet::byteArrayToHex(readSocketData());

    if (!tcpRCVPacket.hexString.trimmed().isEmpty()) {
        QDEBUG() << "Received: " << tcpRCVPacket.hexString;
        emit connectStatus("Received " + QString::number((tcpRCVPacket.hexString.size() / 3) + 1));

        tcpRCVPacket.timestamp = QDateTime::currentDateTime();
        tcpRCVPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        tcpRCVPacket.tcpOrUdp = "TCP";

        if (isSocketEncrypted(*clientSocket())) {
            tcpRCVPacket.tcpOrUdp = "SSL";
        }

        tcpRCVPacket.fromIP = getPeerAddressAsString();

        QDEBUGVAR(tcpRCVPacket.fromIP);
        tcpRCVPacket.toIP = "You";
        tcpRCVPacket.port = sendPacket.fromPort;
        tcpRCVPacket.fromPort = clientSocket()->peerPort();

        if (tcpRCVPacket.hexString.size() > 0) {
            emit packetSent(tcpRCVPacket);

            // Do I need to reply?
            writeResponse(clientConnection, tcpRCVPacket);
        }
    } else {
        QDEBUG() << "No pre-emptive receive data";
    }
}

Packet TCPThread::buildReceivedPacket()
{
    Packet receivedPacket;
    receivedPacket.timestamp = QDateTime::currentDateTime();
    receivedPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
    receivedPacket.tcpOrUdp = "TCP";

    if (isSocketEncrypted(*clientSocket())) {
        QDEBUG() << "Got inside if (isSocketEncrypted(*clientSocket())) in persistentConnectionLoop()";
        receivedPacket.tcpOrUdp = "SSL";
    }

    receivedPacket.fromIP = getPeerAddressAsString();

    QDEBUGVAR(receivedPacket.fromIP);

    receivedPacket.toIP = "You";
    receivedPacket.port = sendPacket.fromPort;
    receivedPacket.fromPort = getPeerPort();

    interruptibleWaitForReadyRead(500);
    emit connectStatus("Waiting to receive");
    receivedPacket.hexString.clear();

    while (clientSocket()->bytesAvailable()) {
        receivedPacket.hexString.append(" ");
        receivedPacket.hexString.append(Packet::byteArrayToHex(clientSocket()->readAll()));
        receivedPacket.hexString = receivedPacket.hexString.simplified();
        interruptibleWaitForReadyRead(100);
    }
    return receivedPacket;
}

quint16 TCPThread::getPeerPort() const
{
    return clientSocket() ? clientSocket()->peerPort() : 0;
}

void TCPThread::handleResponseAfterSend(const Packet& receivedPacket)
{
    // Disconnect now if this is not a persistent connection
    if (!sendPacket.persistent) {
        emit connectStatus("Disconnecting");
        clientSocket()->disconnectFromHost();
    }

    QDEBUG() << "packetSent " << receivedPacket.name << receivedPacket.hexString.size();

    // Emit the received packet (with special case for receiveBeforeSend)
    if (sendPacket.receiveBeforeSend) {
        if (!receivedPacket.hexString.isEmpty()) {
            emit packetSent(receivedPacket);
        }
    } else {
        emit packetSent(receivedPacket);
    }

    // Do I need to reply?
    writeResponse(clientConnection, receivedPacket);

    // Second read: look for any response that came back after our reply
    emit connectStatus("Reading response");

    Packet responsePacket = receivedPacket;  // start with copy
    QByteArray rawData = readSocketData();
    responsePacket.hexString = Packet::byteArrayToHex(rawData);

    responsePacket.timestamp = QDateTime::currentDateTime();
    responsePacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);

    if (responsePacket.hexString.size() > 0) {
        emit packetSent(responsePacket);
        writeResponse(clientConnection, responsePacket);
    }
}

bool TCPThread::shouldBreakPersistentLoop() const
{
    if (!sendPacket.persistent) {
        QDEBUG() << "inside if (!sendPacket.persistent)" ;
        return true;
    }
    return false;
}

void TCPThread::resetPacketForPersistentLoop()
{
    sendPacket.clear();
    /*
     *  clear() resets persistentent to false
     *  But we know we want to go through the loop again
     *  so we have to reassign this variable.
     **/
    sendPacket.persistent = true;
    QDEBUG() << "Persistent connection. Loop and wait.";
}

// THE LOOP
void TCPThread::persistentConnectionLoop()
{
    QDEBUG() << "Entering the forever loop";

    if (closeRequest || isInterruptionRequested()) {
        qDebug() << "Early exit from persistent loop due to close request";
        return;
    }

    while (shouldContinuePersistentLoop()) {
        insidePersistent = true;

        if (closeRequest || isInterruptionRequested()) {  // early exit check (good hygiene)
            qDebug() << "Interruption or close requested - exiting persistent loop";
            QDEBUG() << "closeRequest: " << closeRequest;
            QDEBUG() << "isInterruptionRequested(): " << isInterruptionRequested();
            closeRequest = true;
            break;
        }

        if (sendPacket.hexString.isEmpty() && sendPacket.persistent && (clientSocket()->bytesAvailable() == 0)) {
            handlePersistentIdleCase();
            continue;
        } else {
            QDEBUG() << "IDLE PATH SKIPPED - hexString empty =" << sendPacket.hexString.isEmpty()
                     << " persistent =" << sendPacket.persistent
                     << " bytesAvailable =" << clientSocket()->bytesAvailable();
        }

        const bool disconnected = clientSocket()->state() != QAbstractSocket::ConnectedState;

        // Only treat a disconnection as "broken" if we were expecting the connection to stay open.
        // If this is a non-persistent connection, we expect it to end naturally.
        if (disconnected && sendPacket.persistent) {
            QDEBUG() << "Connection broken.";
            emit connectStatus("Connection broken");

            break;
        }

        if (sendPacket.receiveBeforeSend) {
            handleReceiveBeforeSend();
        }

        sendCurrentPacket();

        Packet receivedPacket = buildReceivedPacket();
        handleResponseAfterSend(receivedPacket);

        if (!sendPacket.persistent) {
            QDEBUG() << "inside if (!sendPacket.persistent)" ;
            break;
        } else {
            sendPacket.clear();
            sendPacket.persistent = true;
            QDEBUG() << "Persistent connection. Loop and wait.";
            continue;
        }
    } // end while connected

    cleanupAfterPersistentConnectionLoop();

} // end persistentConnectionLoop()
