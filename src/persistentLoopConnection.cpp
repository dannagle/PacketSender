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


// THE LOOP
void TCPThread::persistentConnectionLoop()
{
    QDEBUG() << "Entering the forever loop";

    if (closeRequest || isInterruptionRequested()) {
        qDebug() << "Early exit from persistent loop due to close request";
        return;
    }

    int ipMode = 4;
    QHostAddress theAddress(sendPacket.toIP);
    if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
        ipMode = 6;
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

        if (clientSocket()->state() != QAbstractSocket::ConnectedState && sendPacket.persistent) {
            QDEBUG() << "Connection broken.";
            emit connectStatus("Connection broken");

            break;
        }

        if (sendPacket.receiveBeforeSend) {
            QDEBUG() << "Wait for data before sending...";
            emit connectStatus("Waiting for data");
            interruptibleWaitForReadyRead(500);

            Packet tcpRCVPacket;
            tcpRCVPacket.hexString = Packet::byteArrayToHex(clientSocket()->readAll());
            if (!tcpRCVPacket.hexString.trimmed().isEmpty()) {
                QDEBUG() << "Received: " << tcpRCVPacket.hexString;
                emit connectStatus("Received " + QString::number((tcpRCVPacket.hexString.size() / 3) + 1));

                tcpRCVPacket.timestamp = QDateTime::currentDateTime();
                tcpRCVPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
                tcpRCVPacket.tcpOrUdp = "TCP";
                if (clientSocket()->isEncrypted()) {
                    tcpRCVPacket.tcpOrUdp = "SSL";
                }

                if (ipMode < 6) {
                    tcpRCVPacket.fromIP = Packet::removeIPv6Mapping(clientSocket()->peerAddress());
                } else {
                    tcpRCVPacket.fromIP = (clientSocket()->peerAddress()).toString();
                }


                QDEBUGVAR(tcpRCVPacket.fromIP);
                tcpRCVPacket.toIP = "You";
                tcpRCVPacket.port = sendPacket.fromPort;
                tcpRCVPacket.fromPort =    clientSocket()->peerPort();
                if (tcpRCVPacket.hexString.size() > 0) {
                    emit packetSent(tcpRCVPacket);

                    // Do I need to reply?
                    writeResponse(clientConnection, tcpRCVPacket);

                }

            } else {
                QDEBUG() << "No pre-emptive receive data";
            }

        } // end receive before send


        //sendPacket.fromPort = clientSocket()->localPort();
        if(sendPacket.getByteArray().size() > 0) {
            emit connectStatus("Sending data:" + sendPacket.asciiString());
            QDEBUG() << "Attempting write data";
            clientSocket()->write(sendPacket.getByteArray());
            emit packetSent(sendPacket);
        }

        Packet tcpPacket;
        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        tcpPacket.tcpOrUdp = "TCP";
        if (clientSocket()->isEncrypted()) {
            QDEBUG() << "Got inside clientSocket()->isEncrypted() in persistentConnectionLoop()";
            tcpPacket.tcpOrUdp = "SSL";
        }

        if (ipMode < 6) {
            tcpPacket.fromIP = Packet::removeIPv6Mapping(clientSocket()->peerAddress());

        } else {
            tcpPacket.fromIP = (clientSocket()->peerAddress()).toString();

        }
        QDEBUGVAR(tcpPacket.fromIP);

        tcpPacket.toIP = "You";
        tcpPacket.port = sendPacket.fromPort;
        tcpPacket.fromPort =    clientSocket()->peerPort();

        interruptibleWaitForReadyRead(500);
        emit connectStatus("Waiting to receive");
        tcpPacket.hexString.clear();

        while (clientSocket()->bytesAvailable()) {
            tcpPacket.hexString.append(" ");
            tcpPacket.hexString.append(Packet::byteArrayToHex(clientSocket()->readAll()));
            tcpPacket.hexString = tcpPacket.hexString.simplified();
            interruptibleWaitForReadyRead(100);
        }


        if (!sendPacket.persistent) {
            emit connectStatus("Disconnecting");
            clientSocket()->disconnectFromHost();
        }

        QDEBUG() << "packetSent " << tcpPacket.name << tcpPacket.hexString.size();

        if (sendPacket.receiveBeforeSend) {
            if (!tcpPacket.hexString.isEmpty()) {
                emit packetSent(tcpPacket);
            }
        } else {
            emit packetSent(tcpPacket);
        }

        // Do I need to reply?
        writeResponse(clientConnection, tcpPacket);


        emit connectStatus("Reading response");
        tcpPacket.hexString  = clientSocket()->readAll();

        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);


        if (tcpPacket.hexString.size() > 0) {
            emit packetSent(tcpPacket);

            // Do I need to reply?
            writeResponse(clientConnection, tcpPacket);

        }



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
