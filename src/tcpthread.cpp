/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */

#include "tcpthread.h"
#include "globals.h"
#include "packet.h"
#include <QDataStream>
#include <QDebug>
#include <QSettings>
#include <QDesktopServices>
#include <QDir>

#include <QSsl>
#include <QSslKey>
#include <QSslSocket>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QStandardPaths>


TCPThread::TCPThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{

    init();
    sendFlag = false;
    incomingPersistent = false;
    sendPacket.clear();
    insidePersistent = false;
    isSecure = false;
    packetReply.clear();
    consoleMode =  false;


}

TCPThread::TCPThread(Packet sendPacket, QObject *parent)
    : QThread(parent), sendPacket(sendPacket)
{
    sendFlag = true;
    incomingPersistent = false;
    insidePersistent = false;
    isSecure = false;
    consoleMode =  false;
}

// Helper – called from all constructors that create/use a socket
void TCPThread::wireupSocketSignals()
{
    if (!clientConnection) {
        qWarning() << "setupSocketConnections called but clientConnection is null";
        return;
    }

    connect(clientConnection, &QAbstractSocket::connected, this, &TCPThread::onConnected);
    connect(clientConnection, &QAbstractSocket::errorOccurred, this, &TCPThread::onSocketError);
    connect(clientConnection, &QAbstractSocket::stateChanged, this, &TCPThread::onStateChanged);

    // Add any other common connects here in the future (bytesWritten, readyRead, etc.)
    // Example:
    // connect(clientConnection, &QAbstractSocket::readyRead, this, &TCPThread::onReadyRead);
    // connect(clientConnection, &QAbstractSocket::bytesWritten, this, &TCPThread::onBytesWritten);
}
TCPThread::TCPThread(const QString &host, quint16 port,
                     const Packet &initialPacket,
                     QObject *parent)
    : QThread(parent)
    , sendFlag(true)
    , incomingPersistent(false) // treat like client persistent send
    , isSecure(false)
    , consoleMode(false)
    , sendPacket(initialPacket) // set later if SSL
    , insidePersistent(false)
    , m_managedByConnection(true)
{
    // Create socket (use QSslSocket if you plan to support SSL here)
    clientConnection = new QSslSocket(this);

    // Connect signals for tracking
    connect(clientConnection, &QAbstractSocket::connected,
            this, &TCPThread::onConnected);           // add slot if needed
    connect(clientConnection, &QAbstractSocket::errorOccurred,
            this, &TCPThread::onSocketError);
    connect(clientConnection, &QAbstractSocket::stateChanged,
            this, &TCPThread::onStateChanged);

    // Store host/port for run()
    this->host = host;   // add QString host; quint16 port; as private members
    this->port = port;

    qDebug() << "TCPThread (managed client) created for" << host << ":" << port;
}

// SLOTS
void TCPThread::onConnected()
{
    QDEBUG() << "TCPThread: Connected to" << clientConnection->peerAddress().toString() << ":" << clientConnection->peerPort();

    emit connectStatus("Connected");

    // If this is a client persistent connection, start sending/receiving loop
    if (sendFlag) {
        persistentConnectionLoop();
    }
}

void TCPThread::onSocketError(QAbstractSocket::SocketError socketError)
{
    QString errMsg = clientConnection ? clientConnection->errorString() : "Unknown socket error";
    qWarning() << "TCPThread: Socket error" << socketError << "-" << errMsg;

    emit error(socketError);
    emit connectStatus("Error: " + errMsg);

    // Optional: close and clean up
    if (clientConnection) {
        clientConnection->close();
    }
}

void TCPThread::onStateChanged(QAbstractSocket::SocketState state)
{
    QString stateStr;
    switch (state) {
    case QAbstractSocket::UnconnectedState: stateStr = "Unconnected"; break;
    case QAbstractSocket::HostLookupState:  stateStr = "Host Lookup"; break;
    case QAbstractSocket::ConnectingState:  stateStr = "Connecting"; break;
    case QAbstractSocket::ConnectedState:   stateStr = "Connected"; break;
    case QAbstractSocket::BoundState:       stateStr = "Bound"; break;
    case QAbstractSocket::ClosingState:     stateStr = "Closing"; break;
    case QAbstractSocket::ListeningState:   stateStr = "Listening"; break;
    default: stateStr = "Unknown"; break;
    }

    QDEBUG() << "TCPThread: State changed to" << stateStr;

    emit connectStatus(stateStr);

    // If disconnected unexpectedly and persistent, could try reconnect here
    if (state == QAbstractSocket::UnconnectedState && !closeRequest) {
        // Optional: emit disconnected() or retry logic
    }
}

void TCPThread::sendAnother(Packet sendPacket)
{

    QDEBUG() << "Send another packet to " << sendPacket.port;
    this->sendPacket = sendPacket;

}


void TCPThread::loadSSLCerts(QSslSocket * sock, bool allowSnakeOil)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);


    if (!allowSnakeOil) {

        // set the ca certificates from the configured path
        if (!settings.value("sslCaPath").toString().isEmpty()) {
           // sock->setCaCertificates(QSslCertificate::fromPath(settings.value("sslCaPath").toString()));
        }

        // set the local certificates from the configured file path
        if (!settings.value("sslLocalCertificatePath").toString().isEmpty()) {
            sock->setLocalCertificate(settings.value("sslLocalCertificatePath").toString());
        }

        // set the private key from the configured file path
        if (!settings.value("sslPrivateKeyPath").toString().isEmpty()) {
            sock->setPrivateKey(settings.value("sslPrivateKeyPath").toString());
        }

    } else  {


        QString defaultCertFile = CERTFILE;
        QString defaultKeyFile = KEYFILE;
        QFile certfile(defaultCertFile);
        QFile keyfile(defaultKeyFile);

        /*
        #ifdef __APPLE__
                QString certfileS("/Users/dannagle/github/PacketSender/src/ps.pem");
                QString keyfileS("/Users/dannagle/github/PacketSender/src/ps.key");
        #else
                QString certfileS("C:/Users/danie/github/PacketSender/src/ps.pem");
                QString keyfileS("C:/Users/danie/github/PacketSender/src/ps.key");
        #endif

                defaultCertFile = certfileS;
                defaultKeyFile = keyfileS;
        */

        QDEBUG() << "Loading" << defaultCertFile << defaultKeyFile;

        certfile.open(QIODevice::ReadOnly);
        QSslCertificate certificate(&certfile, QSsl::Pem);
        certfile.close();
        if (certificate.isNull()) {
            QDEBUG() << "Bad cert. delete it?";
        }

        keyfile.open(QIODevice::ReadOnly);
        QSslKey sslKey(&keyfile, QSsl::Rsa, QSsl::Pem);
        keyfile.close();
        if (sslKey.isNull()) {
            QDEBUG() << "Bad key. delete it?";
        }


        sock->setLocalCertificate(certificate);
        sock->setPrivateKey(sslKey);

    }

}

void TCPThread::init()
{

}


void TCPThread::wasdisconnected()
{

    QDEBUG();
}

void TCPThread::writeResponse(QSslSocket *sock, Packet tcpPacket)
{

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    bool sendResponse = settings.value("sendReponse", false).toBool();
    bool sendSmartResponse = settings.value("smartResponseEnableCheck", false).toBool();
    QList<SmartResponseConfig> smartList;
    QString responseData = (settings.value("responseHex", "")).toString();
    int ipMode = settings.value("ipMode", 4).toInt();
    smartList.clear();

    smartList.append(Packet::fetchSmartConfig(1, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(2, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(3, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(4, SETTINGSFILE));
    smartList.append(Packet::fetchSmartConfig(5, SETTINGSFILE));

    QByteArray smartData;
    smartData.clear();


    if(consoleMode) {
        responseData.clear();
        sendResponse = false;
        sendSmartResponse = false;
    }


    if (sendSmartResponse) {
        smartData = Packet::smartResponseMatch(smartList, tcpPacket.getByteArray());
    }


    // This is pre-loaded from command line
    if(!packetReply.hexString.isEmpty()) {

        QString data = Packet::macroSwap(packetReply.asciiString());
        QString hexString = Packet::ASCIITohex(data);
        smartData = Packet::HEXtoByteArray(hexString);
    }


    if (sendResponse || !smartData.isEmpty()) {
        Packet tcpPacketreply;
        tcpPacketreply.timestamp = QDateTime::currentDateTime();
        tcpPacketreply.name = "Reply to " + tcpPacket.timestamp.toString(DATETIMEFORMAT);
        tcpPacketreply.tcpOrUdp = "TCP";
        if (sock->isEncrypted()) {
            tcpPacketreply.tcpOrUdp = "SSL";
        }
        tcpPacketreply.fromIP = "You (Response)";
        if (ipMode < 6) {
            tcpPacketreply.toIP = Packet::removeIPv6Mapping(sock->peerAddress());
        } else {
            tcpPacketreply.toIP = (sock->peerAddress()).toString();
        }
        tcpPacketreply.port = sock->peerPort();
        tcpPacketreply.fromPort = sock->localPort();
        QByteArray data = Packet::HEXtoByteArray(responseData);
        tcpPacketreply.hexString = Packet::byteArrayToHex(data);

        QString testMacro = Packet::macroSwap(tcpPacketreply.asciiString());
        tcpPacketreply.hexString = Packet::ASCIITohex(testMacro);

        if (!smartData.isEmpty()) {
            tcpPacketreply.hexString = Packet::byteArrayToHex(smartData);
        }
        sock->write(tcpPacketreply.getByteArray());
        sock->waitForBytesWritten(2000);
        QDEBUG() << "packetSent " << tcpPacketreply.name << tcpPacketreply.hexString;
        emit packetSent(tcpPacketreply);

    }

}


void TCPThread::persistentConnectionLoop()
{
    QDEBUG() << "Entering the forever loop";
    int ipMode = 4;
    QHostAddress theAddress(sendPacket.toIP);
    if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
        ipMode = 6;
    }

    int count = 0;
    while (clientConnection->state() == QAbstractSocket::ConnectedState && !closeRequest) {
        insidePersistent = true;


        if (sendPacket.hexString.isEmpty() && sendPacket.persistent && (clientConnection->bytesAvailable() == 0)) {
            count++;
            if (count % 10 == 0) {
                //QDEBUG() << "Loop and wait." << count++ << clientConnection->state();
                emit connectStatus("Connected and idle.");
            }
            clientConnection->waitForReadyRead(200);
            continue;
        }

        if (clientConnection->state() != QAbstractSocket::ConnectedState && sendPacket.persistent) {
            QDEBUG() << "Connection broken.";
            emit connectStatus("Connection broken");

            break;
        }

        if (sendPacket.receiveBeforeSend) {
            QDEBUG() << "Wait for data before sending...";
            emit connectStatus("Waiting for data");
            clientConnection->waitForReadyRead(500);

            Packet tcpRCVPacket;
            tcpRCVPacket.hexString = Packet::byteArrayToHex(clientConnection->readAll());
            if (!tcpRCVPacket.hexString.trimmed().isEmpty()) {
                QDEBUG() << "Received: " << tcpRCVPacket.hexString;
                emit connectStatus("Received " + QString::number((tcpRCVPacket.hexString.size() / 3) + 1));

                tcpRCVPacket.timestamp = QDateTime::currentDateTime();
                tcpRCVPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
                tcpRCVPacket.tcpOrUdp = "TCP";
                if (clientConnection->isEncrypted()) {
                    tcpRCVPacket.tcpOrUdp = "SSL";
                }

                if (ipMode < 6) {
                    tcpRCVPacket.fromIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());
                } else {
                    tcpRCVPacket.fromIP = (clientConnection->peerAddress()).toString();
                }


                QDEBUGVAR(tcpRCVPacket.fromIP);
                tcpRCVPacket.toIP = "You";
                tcpRCVPacket.port = sendPacket.fromPort;
                tcpRCVPacket.fromPort =    clientConnection->peerPort();
                if (tcpRCVPacket.hexString.size() > 0) {
                    emit packetSent(tcpRCVPacket);

                    // Do I need to reply?
                    writeResponse(clientConnection, tcpRCVPacket);

                }

            } else {
                QDEBUG() << "No pre-emptive receive data";
            }

        } // end receive before send


        //sendPacket.fromPort = clientConnection->localPort();
        if(sendPacket.getByteArray().size() > 0) {
            emit connectStatus("Sending data:" + sendPacket.asciiString());
            QDEBUG() << "Attempting write data";
            clientConnection->write(sendPacket.getByteArray());
            emit packetSent(sendPacket);
        }

        Packet tcpPacket;
        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);
        tcpPacket.tcpOrUdp = "TCP";
        if (clientConnection->isEncrypted()) {
            tcpPacket.tcpOrUdp = "SSL";
        }

        if (ipMode < 6) {
            tcpPacket.fromIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());

        } else {
            tcpPacket.fromIP = (clientConnection->peerAddress()).toString();

        }
        QDEBUGVAR(tcpPacket.fromIP);

        tcpPacket.toIP = "You";
        tcpPacket.port = sendPacket.fromPort;
        tcpPacket.fromPort =    clientConnection->peerPort();

        clientConnection->waitForReadyRead(500);
        emit connectStatus("Waiting to receive");
        tcpPacket.hexString.clear();

        while (clientConnection->bytesAvailable()) {
            tcpPacket.hexString.append(" ");
            tcpPacket.hexString.append(Packet::byteArrayToHex(clientConnection->readAll()));
            tcpPacket.hexString = tcpPacket.hexString.simplified();
            clientConnection->waitForReadyRead(100);
        }


        if (!sendPacket.persistent) {
            emit connectStatus("Disconnecting");
            clientConnection->disconnectFromHost();
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
        tcpPacket.hexString  = clientConnection->readAll();

        tcpPacket.timestamp = QDateTime::currentDateTime();
        tcpPacket.name = QDateTime::currentDateTime().toString(DATETIMEFORMAT);


        if (tcpPacket.hexString.size() > 0) {
            emit packetSent(tcpPacket);

            // Do I need to reply?
            writeResponse(clientConnection, tcpPacket);

        }



        if (!sendPacket.persistent) {
            break;
        } else {
            sendPacket.clear();
            sendPacket.persistent = true;
            QDEBUG() << "Persistent connection. Loop and wait.";
            continue;
        }
    } // end while connected

    if (closeRequest) {
        clientConnection->close();
        clientConnection->waitForDisconnected(100);
    }

}


void TCPThread::closeConnection()
{
    QDEBUG() << "Closing connection";

    if (clientConnection) {
        clientConnection->close();

        // Only wait if the socket was ever connected or is still trying
        // This prevents the "waitForDisconnected() is not allowed in UnconnectedState" warning
        if (clientConnection->state() != QAbstractSocket::UnconnectedState &&
            clientConnection->state() != QAbstractSocket::ClosingState) {
            if (!clientConnection->waitForDisconnected(1000)) {
                qWarning() << "waitForDisconnected timed out";
            }
            } else {
                qDebug() << "Socket already unconnected or closing - no wait needed";
            }
    } else {
        qWarning() << "closeConnection called with null clientConnection";
    }
}


void TCPThread::run()
{
    closeRequest = false;

    //determine IP mode based on send address.
    int ipMode = 4;
    QHostAddress theAddress(sendPacket.toIP);
    if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
        ipMode = 6;
    }

    if (sendFlag) {
        QDEBUG() << "We are threaded sending!";
        clientConnection = new QSslSocket(nullptr);

        sendPacket.fromIP = "You";
        sendPacket.timestamp = QDateTime::currentDateTime();
        sendPacket.name = sendPacket.timestamp.toString(DATETIMEFORMAT);
        bool portpass = false;

        portpass = clientConnection->bind(); //use random port.
        if (portpass) {
            sendPacket.fromPort = clientConnection->localPort();
        }

        // SSL Version...

        if (sendPacket.isSSL()) {
            QSettings settings(SETTINGSFILE, QSettings::IniFormat);

            loadSSLCerts(clientConnection, false);

            if (ipMode > 4) {
                clientConnection->connectToHostEncrypted(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, QAbstractSocket::IPv6Protocol);

            } else {
                clientConnection->connectToHostEncrypted(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);

            }


            if (settings.value("ignoreSSLCheck", true).toBool()) {
                QDEBUG() << "Telling SSL to ignore errors";
                clientConnection->ignoreSslErrors();
            }


            QDEBUG() << "Connecting to" << sendPacket.toIP << ":" << sendPacket.port;
            QDEBUG() << "Wait for connected finished" << clientConnection->waitForConnected(5000);
            QDEBUG() << "Wait for encrypted finished" << clientConnection->waitForEncrypted(5000);

            QDEBUG() << "isEncrypted" << clientConnection->isEncrypted();

            QList<QSslError> sslErrorsList  = clientConnection->
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                    sslErrors();
#else
                    sslHandshakeErrors();
#endif

            Packet errorPacket = sendPacket;
            if (sslErrorsList.size() > 0) {
                QSslError sError;
                foreach (sError, sslErrorsList) {
                    Packet errorPacket = sendPacket;
                    errorPacket.hexString.clear();
                    errorPacket.errorString = sError.errorString();
                    emit packetSent(errorPacket);
                }
            }

            if (clientConnection->isEncrypted()) {
                QSslCipher cipher = clientConnection->sessionCipher();
                Packet errorPacket = sendPacket;
                errorPacket.hexString.clear();
                errorPacket.errorString = "Encrypted with " + cipher.encryptionMethod();
                emit packetSent(errorPacket);

                errorPacket.hexString.clear();
                errorPacket.errorString = "Authenticated with " + cipher.authenticationMethod();
                QDEBUGVAR(cipher.encryptionMethod());
                emit packetSent(errorPacket);

                errorPacket.hexString.clear();
                errorPacket.errorString = "Peer Cert issued by " +  clientConnection->peerCertificate().issuerInfo(QSslCertificate::CommonName).join("\n");
                QDEBUGVAR(cipher.encryptionMethod());
                emit packetSent(errorPacket);

                errorPacket.hexString.clear();
                errorPacket.errorString = "Our Cert issued by " +  clientConnection->localCertificate().issuerInfo(QSslCertificate::CommonName).join("\n");
                QDEBUGVAR(cipher.encryptionMethod());
                emit packetSent(errorPacket);



            } else {
                Packet errorPacket = sendPacket;
                errorPacket.hexString.clear();
                errorPacket.errorString = "Not Encrypted!";
            }


        } else {


            if (ipMode > 4) {
                clientConnection->connectToHost(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, QAbstractSocket::IPv6Protocol);

            } else {
                clientConnection->connectToHost(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);

            }

            clientConnection->waitForConnected(5000);


        }


        if (sendPacket.delayAfterConnect > 0) {
            QDEBUG() << "sleeping " << sendPacket.delayAfterConnect;
            QObject().thread()->usleep(1000 * sendPacket.delayAfterConnect);
        }

        QDEBUGVAR(clientConnection->localPort());

        if (clientConnection->state() == QAbstractSocket::ConnectedState) {
            emit connectStatus("Connected");
            sendPacket.port = clientConnection->peerPort();
            sendPacket.fromPort = clientConnection->localPort();

            persistentConnectionLoop();

            emit connectStatus("Not connected.");
            QDEBUG() << "Not connected.";

        } else {


            //qintptr sock = clientConnection->socketDescriptor();

            //sendPacket.fromPort = clientConnection->localPort();
            emit connectStatus("Could not connect.");
            QDEBUG() << "Could not connect";
            sendPacket.errorString = "Could not connect";
            emit packetSent(sendPacket);

        }

        QDEBUG() << "packetSent " << sendPacket.name;
        if (clientConnection->state() == QAbstractSocket::ConnectedState) {
            clientConnection->disconnectFromHost();
            clientConnection->waitForDisconnected(1000);
            emit connectStatus("Disconnected.");

        }
        clientConnection->close();

        if (!m_managedByConnection)
        {
            clientConnection->deleteLater();
        }

        return;
    }


    QSslSocket sock;
    sock.setSocketDescriptor(socketDescriptor);

    //isSecure = true;

    if (isSecure) {

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);


        //Do the SSL handshake
        QDEBUG() << "supportsSsl" << sock.supportsSsl();

        loadSSLCerts(&sock, settings.value("serverSnakeOilCheck", true).toBool());

        sock.setProtocol(QSsl::AnyProtocol);

        //suppress prompts
        bool envOk = false;
        const int env = qEnvironmentVariableIntValue("QT_SSL_USE_TEMPORARY_KEYCHAIN", &envOk);
        if ((env == 0)) {
            QDEBUG() << "Possible prompting in Mac";
        }

        if (settings.value("ignoreSSLCheck", true).toBool()) {
            sock.ignoreSslErrors();
        }
        sock.startServerEncryption();
        sock.waitForEncrypted();

        QList<QSslError> sslErrorsList  = sock

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                            .sslErrors();
#else
                            .sslHandshakeErrors();
#endif

        Packet errorPacket;
        errorPacket.init();
        errorPacket.timestamp = QDateTime::currentDateTime();
        errorPacket.name = errorPacket.timestamp.toString(DATETIMEFORMAT);
        errorPacket.toIP = "You";
        errorPacket.port = sock.localPort();
        errorPacket.fromPort = sock.peerPort();
        errorPacket.fromIP = sock.peerAddress().toString();

        if (sock.isEncrypted()) {
            errorPacket.tcpOrUdp = "SSL";
        }


        QDEBUGVAR(sock.isEncrypted());

        QDEBUGVAR(sslErrorsList.size());

        if (sslErrorsList.size() > 0) {

            QSslError sError;
            foreach (sError, sslErrorsList) {
                errorPacket.hexString.clear();
                errorPacket.errorString = sError.errorString();
                emit packetSent(errorPacket);
            }

        }


        if (sock.isEncrypted()) {
            QSslCipher cipher = sock.sessionCipher();
            errorPacket.hexString.clear();
            errorPacket.errorString = "Encrypted with " + cipher.encryptionMethod();
            QDEBUGVAR(cipher.encryptionMethod());
            emit packetSent(errorPacket);

            errorPacket.hexString.clear();
            errorPacket.errorString = "Authenticated with " + cipher.authenticationMethod();
            QDEBUGVAR(cipher.encryptionMethod());
            emit packetSent(errorPacket);

            errorPacket.hexString.clear();
            errorPacket.errorString = "Peer cert issued by " +  sock.peerCertificate().issuerInfo(QSslCertificate::CommonName).join("\n");
            QDEBUGVAR(cipher.encryptionMethod());
            emit packetSent(errorPacket);

            errorPacket.hexString.clear();
            errorPacket.errorString = "Our Cert issued by " +  sock.localCertificate().issuerInfo(QSslCertificate::CommonName).join("\n");
            QDEBUGVAR(cipher.encryptionMethod());
            emit packetSent(errorPacket);


        }


        QDEBUG() << "Errors" << sock


#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                            .sslErrors();
#else
                            .sslHandshakeErrors();
#endif



    }

    connect(&sock, SIGNAL(disconnected()),
            this, SLOT(wasdisconnected()));

    //connect(&sock, SIGNAL(readyRead())

    Packet tcpPacket;
    QByteArray data;

    data.clear();
    tcpPacket.timestamp = QDateTime::currentDateTime();
    tcpPacket.name = tcpPacket.timestamp.toString(DATETIMEFORMAT);
    tcpPacket.tcpOrUdp = sendPacket.tcpOrUdp;

    if (ipMode < 6) {
        tcpPacket.fromIP = Packet::removeIPv6Mapping(sock.peerAddress());
    } else {
        tcpPacket.fromIP = (sock.peerAddress()).toString();
    }

    tcpPacket.toIP = "You";
    tcpPacket.port = sock.localPort();
    tcpPacket.fromPort = sock.peerPort();

    sock.waitForReadyRead(5000); //initial packet
    data = sock.readAll();
    tcpPacket.hexString = Packet::byteArrayToHex(data);
    if (sock.isEncrypted()) {
        tcpPacket.tcpOrUdp = "SSL";
    }
    emit packetSent(tcpPacket);
    writeResponse(&sock, tcpPacket);



    if (incomingPersistent) {
        clientConnection = &sock;
        QDEBUG() << "We are persistent incoming";
        sendPacket =  tcpPacket;
        sendPacket.persistent = true;
        sendPacket.hexString.clear();
        sendPacket.port = clientConnection->peerPort();
        sendPacket.fromPort = clientConnection->localPort();
        persistentConnectionLoop();
    }



    /*

        QDateTime twentyseconds = QDateTime::currentDateTime().addSecs(30);

        while ( sock.bytesAvailable() < 1 && twentyseconds > QDateTime::currentDateTime()) {
            sock.waitForReadyRead();
            data = sock.readAll();
            tcpPacket.hexString = Packet::byteArrayToHex(data);
            tcpPacket.timestamp = QDateTime::currentDateTime();
            tcpPacket.name = tcpPacket.timestamp.toString(DATETIMEFORMAT);
            emit packetSent(tcpPacket);

            writeResponse(&sock, tcpPacket);
        }
    */
    insidePersistent = false;
    sock.disconnectFromHost();
    sock.close();
}

bool TCPThread::isEncrypted()
{
    if (insidePersistent && !closeRequest) {
        return clientConnection->isEncrypted();
    } else {
        return false;
    }
}

bool TCPThread::isValid() const
{
    qDebug() << "TCPThread::isValid() called for thread" << this;

    if (!clientConnection) {
        qWarning() << "  → invalid: clientConnection is null";
        return false;
    }

    qDebug() << "  Socket state:" << clientConnection->state()
             << "error:" << clientConnection->error()
             << "error string:" << clientConnection->errorString()
             << "insidePersistent:" << insidePersistent;

    if (clientConnection->error() != QAbstractSocket::UnknownSocketError &&
        clientConnection->error() != QAbstractSocket::SocketTimeoutError) {
        qWarning() << "  → invalid: serious socket error";
        return false;
        }

    switch (clientConnection->state()) {
    case QAbstractSocket::UnconnectedState:
    case QAbstractSocket::ClosingState:
        if (insidePersistent) {
            qWarning() << "  → invalid: Unconnected + insidePersistent=true";
            return false;
        }
        break;

    case QAbstractSocket::HostLookupState:
    case QAbstractSocket::ConnectingState:
    case QAbstractSocket::ConnectedState:
    case QAbstractSocket::BoundState:
    case QAbstractSocket::ListeningState:
        break;

    default:
        qWarning() << "  → invalid: unknown socket state";
        return false;
    }

    qDebug() << "  → valid";
    return true;
}

void TCPThread::sendPersistant(Packet sendpacket)
{
    if ((!sendpacket.hexString.isEmpty()) && (clientConnection->state() == QAbstractSocket::ConnectedState)) {
        QDEBUGVAR(sendpacket.hexString);
        clientConnection->write(sendpacket.getByteArray());
        sendpacket.fromIP = "You";

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        int ipMode = settings.value("ipMode", 4).toInt();


        if (ipMode < 6) {
            sendpacket.toIP = Packet::removeIPv6Mapping(clientConnection->peerAddress());
        } else {
            sendpacket.toIP = (clientConnection->peerAddress()).toString();
        }

        sendpacket.port = clientConnection->peerPort();
        sendpacket.fromPort = clientConnection->localPort();
        if (clientConnection->isEncrypted()) {
            sendpacket.tcpOrUdp = "SSL";
        }
        emit packetSent(sendpacket);
    }
}
