//
// Created by Tomas Gallucci on 4/3/26.
//

#include <QSignalSpy>

#include "testutils.h"

#include <QTcpServer>

#include "settingnames.h"
#include "globals.h"

void TestUtils::debugSpy(const QSignalSpy& spy)
{
    qDebug() << "QSignalSpy captured" << spy.count() << "emissions";
    for (int i = 0; i < spy.count(); ++i) {
        QList<QVariant> args = spy.at(i);
        qDebug() << "  Emission" << i << "has" << args.size() << "arguments:";
        for (int j = 0; j < args.size(); ++j) {
            qDebug() << "    Arg" << j << ":" << args.at(j)
                     << "(type:" << args.at(j).typeName() << ")";
        }
    }
}

Packet TestUtils::createPacketForTest()
{
    Packet p;

    p.toIP = "127.0.0.1";
    p.port = 666;
    p.hexString = "AA BB CC DD";
    p.tcpOrUdp = "TCP";

    return p;
}

Packet TestUtils::createIdlePacketForTest()
{
    Packet p;

    p.toIP = "127.0.0.1";
    p.port = 666;
    p.hexString.clear();
    p.tcpOrUdp = "TCP";
    p.persistent = true;

    return p;

}

MockSslSocket* TestUtils::createMockSocketForTest()
{
    auto mockSock = new MockSslSocket();

    mockSock->setMockConnected(true);
    mockSock->setIsValid(true);

    return mockSock;
}

QString TestUtils::extractResourceToTempFile(const QString& resourcePath)
{
    QFile resource(resourcePath);
    if (!resource.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open resource:" << resourcePath;
        return {};
    }

    QByteArray data = resource.readAll();
    resource.close();

    if (resourcePath.endsWith(".base64")) {
        data = QByteArray::fromBase64(data);
    }

    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);   // keep file after test
    if (tempFile.open()) {
        tempFile.write(data);
        tempFile.close();
        return tempFile.fileName();
    }

    return {};
}

void TestUtils::setupProductionSnakeOilCertsForTest()
{
    QSettings& settings = getSettings();

    QString certPath = extractResourceToTempFile(SNAKEOIL_BASE64_CERT);
    QString keyPath  = extractResourceToTempFile(SNAKEOIL_BASE64_KEY);

    if (!certPath.isEmpty()) {
        settings.setValue(SET_LOCAL_CERTIFICATE_PATH, certPath);
    }
    if (!keyPath.isEmpty()) {
        settings.setValue(SSL_PRIVATE_KEY_PATH, keyPath);
    }

    settings.setValue(LOAD_SNAKEOIL_CERTS, false);
    settings.sync();
}

bool TestUtils::qStringVectorStartsWith(const std::vector<QString>& vec, const std::vector<QString>& prefix)
{
    if (prefix.size() > vec.size()) return false;
    return std::equal(prefix.begin(), prefix.end(), vec.begin());
}

bool TestUtils::qStringVectorEndsWith(const std::vector<QString>& vec, const std::vector<QString>& suffix)
{
    if (suffix.size() > vec.size()) return false;
    return std::equal(suffix.begin(), suffix.end(), vec.end() - suffix.size());
}

QTcpServer& TestUtils::startQTcpServer()
{
    auto *server = new QTcpServer();
    server->listen(QHostAddress::LocalHost);
    return *server;
}
