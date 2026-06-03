//
// Created by Tomas Gallucci on 5/9/26.
//

#include "outgoingtcpthread.h"

#include <QMetaEnum>
#include <QSslKey>

#include "realqsslsocket.h"
#include "settingnames.h"
#include "fileutils.h"

OutgoingTcpThread::OutgoingTcpThread(PacketSenderQSslSocketInterface* socketInterface,
                                     const Packet& packetToSend,
                                     QObject* parent)
    : BaseTcpThread(socketInterface, parent)
      , sendPacket(packetToSend)
{
    if (packetToSend.toIP.isEmpty()) {
        throw std::invalid_argument("OutgoingTcpThread: packetToSend.toIP cannot be empty");
    }

    if (packetToSend.port <= 0) {
        throw std::invalid_argument("OutgoingTcpThread: packetToSend.port must be set to a positive integer value");
    }

    persistent = packetToSend.persistent;
}

// Convenience constructor
OutgoingTcpThread::OutgoingTcpThread(const Packet& packetToSend,
                                     QObject* parent)
    : OutgoingTcpThread(new RealQSslSocket(new QSslSocket()), packetToSend, parent)   // delegates to main constructor
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

void OutgoingTcpThread::run()
{
    // === 1. Establish connection (plain TCP or SSL) ===
    if (sendPacket.isSSL()) {
        if (!handleOutgoingSSL()) {
            return;                    // SSL handshake failed - errors already emitted
        }
    } else {
        if (!handleOutgoingPlainTCP()) {
            return;                    // plain TCP connect failed - errors already emitted
        }
    }

    // === 2. We are now successfully connected (plain or SSL) ===
    if (sendPacket.delayAfterConnect > 0) {
        QDEBUG() << "sleeping" << sendPacket.delayAfterConnect;
        sleep(1000 * sendPacket.delayAfterConnect);
    }

    prepareOutgoingPacket();
    sendOutgoingPacket();
    processIncomingData();

    // === 3. Decide how to finish the connection ===
    if (persistent) {
        qWarning() << "got inside persistent if statement before loop";
        persistentConnectionLoop();    // pure loop only
    }

    closeConnection();                 // Single cleanup point for both single-shot and persistent
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
    const auto s = getSocketInterface();
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

Packet OutgoingTcpThread::buildReplyPacket(const Packet& receivedPacket,
                                           const QByteArray& responseData)
{
    Packet reply;

    reply.timestamp = QDateTime::currentDateTime();
    reply.name = "Reply to " + receivedPacket.timestamp.toString(DATETIMEFORMAT);

    reply.fromIP = "You (Response)";

    // Safe reversal with fallbacks
    reply.toIP = !receivedPacket.fromIP.isEmpty()
                     ? receivedPacket.fromIP
                     : sendPacket.toIP;                    // fallback

    reply.port = (receivedPacket.fromPort > 0)
                     ? receivedPacket.fromPort
                     : sendPacket.port;                    // fallback

    reply.fromPort = getSocketInterface()->getLocalPort();

    reply.tcpOrUdp = receivedPacket.tcpOrUdp;
    if (isSocketEncrypted()) {
        reply.tcpOrUdp = "SSL";
    }

    // Response content
    // === Smart Response (higher priority) ===
    QByteArray smartData = getSmartResponseData(receivedPacket);

    if (!smartData.isEmpty()) {
        reply.hexString = Packet::byteArrayToHex(smartData);
    }  else if (!responseData.isEmpty()) {
        reply.hexString = Packet::byteArrayToHex(responseData);
    } else
    {
        const QSettings& settings = getSettings();
        reply.hexString = settings.value(RESPONSE_HEX).toString();
    }

    // Macro expansion
    QString expanded = Packet::macroSwap(reply.asciiString());
    reply.hexString = Packet::ASCIITohex(expanded);

    return reply;
}

void OutgoingTcpThread::loadSnakeOilCertificate()
{
    const QByteArray decoded = FileUtils::decodeBase64EncodedResourceFile(SNAKEOIL_BASE64_CERT);
    const QSslCertificate certificate(decoded, QSsl::Pem);

    if (!certificate.isNull() && getSocketInterface()) {
        getSocketInterface()->setLocalCertificate(certificate);
    }
}

void OutgoingTcpThread::loadSnakeOilKey()
{
    const QByteArray decoded = FileUtils::decodeBase64EncodedResourceFile(SNAKEOIL_BASE64_KEY);
    const QSslKey sslKey(decoded, QSsl::Rsa, QSsl::Pem);

    if (!sslKey.isNull() && getSocketInterface()) {
        getSocketInterface()->setPrivateKey(sslKey);
    }
}

void OutgoingTcpThread::loadSnakeOilCerts()
{
    // Certificate
    loadSnakeOilCertificate();

    // Private Key
    loadSnakeOilKey();
}

void OutgoingTcpThread::loadSSLCerts(bool allowSnakeOil)
{
    auto sock = getSocketInterface();

    if (!sock) {
        emit errorMessage("loadSSLCerts called with null socketInterface");
        return;
    }

    if (allowSnakeOil)
    {
        loadSnakeOilCerts();
        return;
    }

    const QSettings& settings = getSettings();

    // Production / user-provided certs
    QString certPath = settings.value(SET_LOCAL_CERTIFICATE_PATH).toString();
    QString keyPath  = settings.value(SSL_PRIVATE_KEY_PATH).toString();

    if (!certPath.isEmpty()) {
        sock->setLocalCertificate(certPath);

        if (!sock->hasLocalCertificate()) {
            emit errorMessage("SSL: Failed to load certificate from: " + certPath);
        }
    }
    if (!keyPath.isEmpty()) {
        sock->setPrivateKey(keyPath);

        if (!sock->hasPrivateKey()) {
            emit errorMessage("SSL: Failed to load private key from: " + keyPath);
        }
    }
}

void OutgoingTcpThread::handleOutgoingSSLHandshakeSuccess()
{
    auto sslSocket = getSocketInterface();
    if (!sslSocket) return;

    QSslCipher cipher = sslSocket->sessionCipher();

    Packet info = sendPacket;
    info.hexString.clear();

    info.errorString = "Encrypted with " + cipher.encryptionMethod();
    emit packetSent(info);

    info.errorString = "Authenticated with " + cipher.authenticationMethod();
    emit packetSent(info);

    info.errorString = "Peer Cert: " +
        sslSocket->peerCertificate().issuerInfo(QSslCertificate::CommonName).join(", ");
    emit packetSent(info);

    QDEBUG() << "SSL handshake successful - cipher:" << cipher.name();
}

void OutgoingTcpThread::handleOutgoingSSLHandshakeFailure()
{
    QDEBUG() << "SSL handshake failed";

    Packet errorPacket = sendPacket;
    errorPacket.hexString.clear();
    errorPacket.errorString.clear();

    bool hadSpecificErrors = false;

    auto sslSocket = getSocketInterface();
    if (sslSocket) {
        QList<QSslError> errors =
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            sslSocket->sslErrors();
#else
            sslSocket->sslHandshakeErrors();
#endif

        for (const QSslError& err : errors) {
            QString msg = "SSL Error: " + err.errorString();

            emit errorMessage(msg);
            QDEBUG() << msg;

            // Still show each concrete SSL error in the packet log (preserves Dan's UI intent)
            errorPacket.errorString = msg;
            emit packetSent(errorPacket);

            hadSpecificErrors = true;
        }
    }

    // Summary (only once, outside the loop)
    QString summary = "SSL Handshake Failed";
    emit errorMessage(summary);
    emit error(QSslSocket::SslHandshakeFailedError);   // low-level signal once

    if (!hadSpecificErrors) {
        errorPacket.errorString = summary;
        emit packetSent(errorPacket);
    }

    // Note: We do *not* emit a generic "SSL connection failed" here.
    // That can be emitted at a higher level if needed (e.g. in handleOutgoingSSL()).
}

bool OutgoingTcpThread::handleOutgoingSSL()
{
    QDEBUG() << "Starting SSL connection to" << sendPacket.toIP << ":" << sendPacket.port;
    const auto sslSocketInterface = getSocketInterface();
    if (!sslSocketInterface) {
        qWarning() << "Failed to get QSslSocket";
        handleConnectionFailure();
        return false;
    }

    // Load certificates / keys
    loadSSLCerts(getSettings().value(LOAD_SNAKEOIL_CERTS, true).toBool());

    sslSocketInterface->setProtocol(QSsl::AnyProtocol);

    const QSettings& settings = getSettings();
    if (settings.value(IGNORE_SSL_CHECK, true).toBool()) {
        sslSocketInterface->ignoreSslErrors();
    }

    sslSocketInterface->connectToHostEncrypted(sendPacket.toIP,
                                      sendPacket.port,
                                      QIODevice::ReadWrite,
                                      getIPConnectionProtocol());

    bool connected = sslSocketInterface->waitForConnected(5000);
    bool encrypted = sslSocketInterface->waitForEncrypted(5000);

    outgoingConnectionDebugMessage(connected && encrypted);

    if (connected && encrypted) {
        emit connectionStatus("SSL Connected");
        handleOutgoingSSLHandshakeSuccess();
        return true;
    } else {
        handleOutgoingSSLHandshakeFailure();
        return false;
    }
}

bool OutgoingTcpThread::handleOutgoingPlainTCP()
{
    getSocketInterface()->connectToHost(sendPacket.toIP,
                               sendPacket.port,
                               QIODevice::ReadWrite,
                               getIPConnectionProtocol());

    bool success = getSocketInterface()->waitForConnected(5000);
    outgoingConnectionDebugMessage(success);

    success ? emit connectionStatus("Connected") : handleConnectionFailure();
    return success;
}

void OutgoingTcpThread::handleConnectionFailure()
{
    emit connectionStatus("Could not connect.");
    emit errorMessage("Could not connect to " + sendPacket.toIP + ":" + QString::number(sendPacket.port));

    sendPacket.errorString = "Could not connect";
    emit packetSent(sendPacket);
}

bool OutgoingTcpThread::shouldContinuePersistentLoop() const
{
    QDEBUG() << "\nshouldContinuePersistentLoop()\n"
        << "isInterruptionRequested(): " << isInterruptionRequested() << "\n"
        << "getSocketInterface(): " << getSocketInterface() << "\n"
        << "getSocketState(): " << getSocketState() << "\n"
        << "getSocketState() == QAbstractSocket::ConnectedState: " << (getSocketState() == QAbstractSocket::ConnectedState) << "\n"
        << "!shouldStop(): " << !shouldStop() << "\n";

    bool result =  !shouldStop() &&
        getSocketInterface() &&
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
        << "bytesAvailable =" << (getSocketInterface() ? getSocketInterface()->bytesAvailable() : -1);

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
    p.fromPort = getSocketInterface() ? getSocketInterface()->getLocalPort() : 0;

    if (getSocketInterface()) {
        QByteArray data = readSocketData();
        p.hexString = Packet::byteArrayToHex(data);
    }

    return p;
}

QByteArray OutgoingTcpThread::getSmartResponseData(const Packet& receivedPacket)
{
    const QSettings& settings = getSettings();

    if (!settings.value(SMART_RESPONSES_ENABLED, false).toBool()) {
        return {};
    }

    QList<SmartResponseConfig> smartList;
    for (int i = 1; i <= 5; ++i) {
        smartList.append(Packet::fetchSmartConfig(i, settings));
    }

    Packet nonConstPacket = receivedPacket;
    QByteArray smartData = Packet::smartResponseMatch(smartList, nonConstPacket.getByteArray());

    QDEBUG() << "SmartResponseMatch input hex :" << receivedPacket.hexString;
    QDEBUG() << "SmartResponseMatch result size:" << smartData.size();
    QDEBUG() << "SmartResponseMatch result hex :" << Packet::byteArrayToHex(smartData);

    if (!smartData.isEmpty()) {
        QDEBUG() << "Smart response matched";
    }

    return smartData;
}

void OutgoingTcpThread::processIncomingData()
{
    if (!getSocketInterface() || getSocketInterface()->bytesAvailable() == 0) {
        return;
    }

    QDEBUG() << "past early return";

    Packet received = buildReceivedPacket();
    if (!received.hexString.isEmpty()) {
        emit packetReceived(received);

        // TODO: Smart response logic will go here later
        sendReplyIfNeeded(received);
    }
}

void OutgoingTcpThread::waitForAndProcessIncomingData()
{
    QDEBUG() << "receiveBeforeSend mode: waiting for incoming data...";
    emit connectionStatus("Waiting for data before send");

    // Wait for data (interruptible)
    interruptibleWaitForReadyRead(500);

    processIncomingData();

    // Optionally: auto-send a response here if configured
}

bool OutgoingTcpThread::shouldSendReply() const
{
    const QSettings &settings = getSettings();

    bool basicResponseEnabled = settings.value(SEND_RESPONSE, false).toBool();
    bool smartResponseEnabled = settings.value(SMART_RESPONSES_ENABLED, false).toBool();
    bool hasCommandLineReply  = !commandLineReplyPacket.hexString.isEmpty();

    if (consoleMode)
    {
        return hasCommandLineReply;
    }

    // If either basic response or smart responses are enabled
    return basicResponseEnabled || smartResponseEnabled || hasCommandLineReply;
}

void OutgoingTcpThread::sendReplyIfNeeded(const Packet& receivedPacket)
{
    if (!shouldSendReply()) {
        QDEBUG() << "No reply configured - skipping";
        return;
    }

    QDEBUG() << "shouldSendReply() == true → building reply";

    // Smart response data will go here in the future
    QByteArray responseData;

    Packet reply = buildReplyPacket(receivedPacket, responseData);

    // Command-line reply has highest priority
    if (!commandLineReplyPacket.hexString.isEmpty()) {
        reply = commandLineReplyPacket;
        QDEBUG() << "Using command-line reply packet instead";
    }

    // Don't send empty replies (this was in the old writeResponse)
    if (reply.hexString.isEmpty()) {
        QDEBUG() << "Reply has no data - skipping send";
        return;
    }

    // === Actual send via Base class ===
    BaseTcpThread::sendOutgoingPacket(reply);
}

void OutgoingTcpThread::persistentConnectionLoop()
{
    QDEBUG() << "Entering persistent connection loop for" << sendPacket.toIP << ":" << sendPacket.port;

    while (shouldContinuePersistentLoop()) {
        if (shouldStopPersistentConnectionLoop()) {
            break;
        }

        const bool isIdleCondition = sendPacket.hexString.isEmpty() &&
            sendPacket.persistent &&
            getSocketInterface()->bytesAvailable() == 0;
        idleDebugMessage(isIdleCondition);

        if (sendPacket.receiveBeforeSend)
        {
            waitForAndProcessIncomingData();
        } else if (isIdleCondition)
        {
            handlePersistentIdleCase(2000);
        }
        else {
            processIncomingData();
        }
    }

    QDEBUG() << "Exiting persistent connection loop";
}

void OutgoingTcpThread::outgoingConnectionDebugMessage(const bool connectSuccess)
{
    const auto s = getSocketInterface();
    qDebug() << "[OutgoingTcpThread plain connect] ========================================";
    qDebug() << "  waitForConnected() returned:" << connectSuccess;
    qDebug() << "  socket state:" << s->getSocketState();
    qDebug() << "  socket error:" << s->getErrorString();
    qDebug() << "  peer:" << s->getPeerAddress().toString() << ":" << s->getPeerPort();
    qDebug() << "  local port:" << s->getLocalPort();
    qDebug() << "================================================================";
}

void OutgoingTcpThread::idleDebugMessage(bool isIdleCondition) const
{
    QDEBUG() << "Idle condition check:"
        << "hexString.empty() =" << sendPacket.hexString.isEmpty()
        << "persistent =" << sendPacket.persistent
        << "bytesAvailable() =" << getSocketInterface()->bytesAvailable()
        << "→ isIdleCondition =" << isIdleCondition;
}
