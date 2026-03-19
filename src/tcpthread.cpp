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

// Helper – called from all Connection-managed constructors that create/use a socket
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

// Client / outgoing persistent constructor
TCPThread::TCPThread(const QString &host, quint16 port,
                     const Packet &initialPacket,
                     QObject *parent)
    : QThread(parent)
    , sendFlag(true)
    , incomingPersistent(false) // treat like client persistent send
    , isSecure(false)
    , consoleMode(false)
    , socketDescriptor(-1) // set later if SSL
    , sendPacket(initialPacket)
    , insidePersistent(false)
    , host(host) // Store host for run()
    , port(port) // Store port for run()
    , m_managedByConnection(true)
{
    qDebug() << "NEW CONSTRUCTOR CALLED with host:" << host;

    // Create socket (use QSslSocket if you plan to support SSL here)
    clientConnection = new QSslSocket(this);

    // Connect signals for tracking
    wireupSocketSignals();

    sendPacket.toIP = host;          // ← make run() use the passed host
    sendPacket.port = port;          // ← make run() use the passed port
    qDebug() << "Constructor set sendPacket.toIP =" << sendPacket.toIP
             << "port =" << sendPacket.port;

    qDebug() << "TCPThread (managed client) created for" << host << ":" << port;
}

// Incoming / server constructor
TCPThread::TCPThread(int socketDescriptor,
                     bool isSecure,
                     bool isPersistent,
                     QObject *parent)
    : QThread(parent)
    , sendFlag(false)               // no auto-send on accept
    , incomingPersistent(isPersistent)
    , isSecure(isSecure)
    , consoleMode(false)
    , socketDescriptor(socketDescriptor)
    , insidePersistent(false)
    , m_managedByConnection(true)
{
    clientConnection = new QSslSocket(this);  // Always QSslSocket — works for plain TCP too

    // Choose socket type based on isSecure
    if (isSecure) {
        // TODO: Load server certificate / private key here
        /* If isSecure == true, prepare for server-side encryption (deferred to run() or init)
         * For now, just log the intent
         */
        qDebug() << "Incoming secure connection requested — server SSL setup pending in run()";
        // e.g. clientConnection->setLocalCertificate(...);
        // clientConnection->setPrivateKey(...);
    } // else {
        // clientConnection = new QTcpSocket(this);
    // }

    // host and port unused in incoming mode — left to defaults

    wireupSocketSignals();

    qDebug() << "TCPThread (incoming) created with descriptor" << socketDescriptor
             << (isSecure ? " (SSL)" : " (plain)")
             << (isPersistent ? " - persistent" : "");
}

TCPThread::~TCPThread()
{
    if (isRunning()) {
        qDebug() << "TCPThread destructor: requesting interruption and waiting...";
        requestInterruption();           // tell run() to stop
        quit();                          // if using exec(), stop event loop

            qDebug() << "TCPThread destructor: waiting " << destructorWaitMs << " ms...";
        if (!wait(destructorWaitMs)) {               // give it 5 seconds in production, 500 ms in unit tests
            qWarning() << "TCPThread did not finish in time during destruction - terminating!";
            terminate();                 // last resort (not ideal, but better than crash)
        }
    }

}

void TCPThread::forceShutdown()
{
    closeRequest = true;
    requestInterruption();

    // If we're blocked in waitForReadyRead, abort the socket to unblock
    if (clientConnection && clientConnection->state() == QAbstractSocket::ConnectedState) {
        clientConnection->abort();  // immediately unblocks waitFor* calls
        qDebug() << "forceShutdown: aborted socket to unblock waits";
    }
}

bool TCPThread::interruptibleWaitForReadyRead(const int timeoutMs) const
{
    const int chunk = 50;  // check every 50 ms
    int remaining = divideWaitBy10ForUnitTest() ? timeoutMs / 10 : timeoutMs;

    QDEBUG() << "initial remaining: " << remaining;

    while (remaining > 0 && !isInterruptionRequested()) {
        if (clientConnection->waitForReadyRead(chunk)) {
            QDEBUG() << "inside if waitForReadyRead(chunk)";
            return true;
        }
        remaining -= chunk;
        QDEBUG() << "remaining after substraction: " << remaining;
        QThread::msleep(1);  // tiny yield
    }

    return false;
}

QSslSocket* TCPThread::clientSocket()
{
    if (!clientConnection) {
        QDEBUG() << "clientSocket: lazy creation of real socket";
        clientConnection = new QSslSocket(this);
        clientSocket()->setParent(this);
        wireupSocketSignals();
    }

    return clientConnection;
}
// EXTRACTIONS FROM run()

QAbstractSocket::NetworkLayerProtocol TCPThread::getIPConnectionProtocol() const
{
    // Primary source: sendPacket.toIP (matches original run() logic)
    QHostAddress packetAddr(sendPacket.toIP);
    QAbstractSocket::NetworkLayerProtocol protocol =
        (packetAddr.protocol() == QAbstractSocket::IPv6Protocol)
            ? QAbstractSocket::IPv6Protocol
            : QAbstractSocket::IPv4Protocol;

    // Defensive check: warn if host disagrees (host is actual connect target)
    QHostAddress hostAddr(host);
    QAbstractSocket::NetworkLayerProtocol hostProtocol =
        (hostAddr.protocol() == QAbstractSocket::IPv6Protocol)
            ? QAbstractSocket::IPv6Protocol
            : QAbstractSocket::IPv4Protocol;

    if (protocol != hostProtocol && !host.isEmpty() && !sendPacket.toIP.isEmpty()) {
        qWarning().nospace()
            << "IP protocol mismatch: sendPacket.toIP indicates "
            << protocol << " but host indicates " << hostProtocol
            << " (using sendPacket.toIP)";
    }

    return protocol;
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
        clientConnection->state() == QAbstractSocket::ConnectedState && !closeRequest) {
        insidePersistent = true;

        if (closeRequest || isInterruptionRequested()) {  // early exit check (good hygiene)
            qDebug() << "Interruption or close requested - exiting persistent loop";
            QDEBUG() << "closeRequest: " << closeRequest;
            QDEBUG() << "isInterruptionRequested(): " << isInterruptionRequested();
            closeRequest = true;
            break;
        }

        if (sendPacket.hexString.isEmpty() && sendPacket.persistent && (clientConnection->bytesAvailable() == 0)) {
            count++;
            if (count % 10 == 0) {
                //QDEBUG() << "Loop and wait." << count++ << clientConnection->state();
                emit connectStatus("Connected and idle.");
            }
            interruptibleWaitForReadyRead(200);
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
            interruptibleWaitForReadyRead(500);

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
            QDEBUG() << "Got inside clientConnection->isEncrypted() in persistentConnectionLoop()";
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

        interruptibleWaitForReadyRead(500);
        emit connectStatus("Waiting to receive");
        tcpPacket.hexString.clear();

        while (clientConnection->bytesAvailable()) {
            tcpPacket.hexString.append(" ");
            tcpPacket.hexString.append(Packet::byteArrayToHex(clientConnection->readAll()));
            tcpPacket.hexString = tcpPacket.hexString.simplified();
            interruptibleWaitForReadyRead(100);
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
        if (clientConnection->state() == QAbstractSocket::ConnectedState ||
            clientConnection->state() == QAbstractSocket::ClosingState) {
            clientConnection->disconnectFromHost();
            clientConnection->waitForDisconnected(500);  // shorter timeout is fine here
        }

        clientConnection->close();

        if (!m_managedByConnection) {
            clientConnection->deleteLater();
        }
        clientConnection = nullptr;  // clear pointer
    }

    emit connectStatus("Disconnected");

} // end persistentConnectionLoop()


void TCPThread::closeConnection()
{
    QDEBUG() << "closeConnection requested from" << (QThread::currentThread() == this ? "worker" : "main/other");

    closeRequest = true;               // worker loop checks this
    requestInterruption();             // for any interruptible waits

    // Do NOT call clientConnection->close() here — worker will do it
}


void TCPThread::run()
{
    QAbstractSocket::NetworkLayerProtocol ipConnectionProtocol = getIPConnectionProtocol();

    if (sendFlag) {
        QDEBUG() << "We are threaded sending!";
        clientConnection = new QSslSocket(nullptr);

        // Use the constructor-passed host/port instead of sendPacket
        QString connectHost = host.isEmpty() ? sendPacket.toIP : host;
        quint16 connectPort = (port > 0) ? port : sendPacket.port;

        qDebug() << "Connecting using host:" << connectHost << "port:" << connectPort;

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
            clientConnection->connectToHostEncrypted(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, ipConnectionProtocol);


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
            clientConnection->connectToHost(sendPacket.toIP,  sendPacket.port, QIODevice::ReadWrite, ipConnectionProtocol);

            bool connectSuccess = clientConnection->waitForConnected(5000);

            qDebug() << "[TCPThread client connect] ========================================";
            qDebug() << "  waitForConnected() returned:" << connectSuccess;
            qDebug() << "  socket state:" << clientConnection->state();
            qDebug() << "  socket error code:" << clientConnection->error();
            qDebug() << "  socket error string:" << clientConnection->errorString();
            qDebug() << "  peer:" << clientConnection->peerAddress().toString() << ":" << clientConnection->peerPort();
            qDebug() << "  local port:" << clientConnection->localPort();
            qDebug() << "================================================================";


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


    tcpPacket.fromIP = ipConnectionProtocol == QAbstractSocket::IPv6Protocol ?
        Packet::removeIPv6Mapping(sock.peerAddress()) : (sock.peerAddress()).toString();

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
        clientConnection = new QSslSocket(this);

        if (!clientConnection->setSocketDescriptor(socketDescriptor)) {
            qWarning() << "Failed to set socket descriptor on clientConnection";
            delete clientConnection;
            clientConnection = nullptr;
            return;
        }

        // ... copy any state from sock if needed (e.g. encryption state)
        QDEBUG() << "Persistent incoming mode entered - using heap clientConnection";
        sendPacket = tcpPacket;
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
