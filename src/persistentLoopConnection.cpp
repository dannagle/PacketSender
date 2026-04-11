//
// Created by Tomas Gallucci on 4/11/26.
//

// EXTRACTED FROM TcpThread

#include "tcpthread.h"

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

    int count = 0;
    while (!isInterruptionRequested() &&
        clientSocket()->state() == QAbstractSocket::ConnectedState && !closeRequest) {
        insidePersistent = true;

        if (closeRequest || isInterruptionRequested()) {  // early exit check (good hygiene)
            qDebug() << "Interruption or close requested - exiting persistent loop";
            QDEBUG() << "closeRequest: " << closeRequest;
            QDEBUG() << "isInterruptionRequested(): " << isInterruptionRequested();
            closeRequest = true;
            break;
        }

        if (sendPacket.hexString.isEmpty() && sendPacket.persistent && (clientSocket()->bytesAvailable() == 0)) {
            count++;
            if (count % 10 == 0) {
                //QDEBUG() << "Loop and wait." << count++ << clientSocket()->state();
                emit connectStatus("Connected and idle.");
            }
            interruptibleWaitForReadyRead(200);
            continue;
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

    qDebug() << "persistentConnectionLoop exiting - cleaning up socket";

    if (clientConnection) {
        if (clientSocket()->state() == QAbstractSocket::ConnectedState ||
            clientSocket()->state() == QAbstractSocket::ClosingState) {
            clientSocket()->disconnectFromHost();
            clientSocket()->waitForDisconnected(500);  // shorter timeout is fine here
        }

        clientSocket()->close();

        if (!m_managedByConnection) {
            clientSocket()->deleteLater();
        }
        clientConnection = nullptr;  // clear pointer
    }

    emit connectStatus("Disconnected");

} // end persistentConnectionLoop()
