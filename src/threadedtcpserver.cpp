#include "threadedtcpserver.h"

#include <QDebug>
#include <QSslSocket>
#include <QHostAddress>
#include <QSslConfiguration>

#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QtGlobal>

#include "globals.h"
#include "tcpthread.h"

ThreadedTCPServer::ThreadedTCPServer(QObject *parent) :
    QTcpServer(parent)
{

    /*
    QDEBUG() << "sslserver bind: " << listen(
                    QHostAddress::AnyIPv4
                    , 5100);

                    */

}

void ThreadedTCPServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket * sslSock = new QSslSocket(this);
    QDEBUG() << "supportsSsl" << sslSock->supportsSsl();

    sslSock->setSocketDescriptor(socketDescriptor);


    QFile certfile("/Users/dannagle/github/PacketSender/src/ps.pem");
    QFile keyfile("/Users/dannagle/github/PacketSender/src/ps.key");

    //suppress prompts
    bool envOk = false;
    const int env = qEnvironmentVariableIntValue("QT_SSL_USE_TEMPORARY_KEYCHAIN", &envOk);
    if((env == 0)) {
        QDEBUG() << "Possible prompting in Mac";
    }


    certfile.open (QIODevice::ReadOnly);
    QSslCertificate certificate (&certfile, QSsl::Pem);
    certfile.close ();

    keyfile.open (QIODevice::ReadOnly);
    QSslKey sslKey (&keyfile, QSsl::Rsa, QSsl::Pem);
    keyfile.close ();


    sslSock->setLocalCertificate(certificate);
    sslSock->setPrivateKey(sslKey);

    //sslSock->setSslConfiguration(TCPThread::loadSSLCerts(true));

    sslSock->setProtocol( QSsl::AnyProtocol );
    sslSock->ignoreSslErrors();
    sslSock->startServerEncryption();
    sslSock->waitForEncrypted();
    QDEBUGVAR(sslSock->isEncrypted());
    QDEBUG() << "Errors" << sslSock->sslErrors();

    sslSock->write(QByteArray("sslSock was sent."));
    sslSock->waitForBytesWritten();
    sslSock->close();

}


