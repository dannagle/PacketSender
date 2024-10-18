#pragma once

#include <QThread>
#include "packet.h"
#include "globals.h"
#include "QSettings"
#include "association.h"
//#include "QTimer"

#if QT_VERSION > QT_VERSION_CHECK(6, 00, 0)


class Dtlsthread : public QThread
{
    Q_OBJECT

public:
    bool respondRecieved = false;
    int retries = 0;
    Dtlsthread(Packet sendPacket, QObject *parent);
    DtlsAssociation* initDtlsAssociation();
    virtual ~Dtlsthread();
    void persistentConnectionLoop();
    void run() override; // Pure virtual function making this class abstract
    std::vector<QString> getCmdInput(Packet sendpacket, QSettings& settings);
    void writeMassage(Packet packetToSend, DtlsAssociation* dtlsAssociation);
    void retryHandshake();

    QTimer* timer;
    std::vector<DtlsAssociation*> dtlsAssociations;
    DtlsAssociation* dtlsAssociation;
    QString recievedMassage;
    Packet sendpacket;


    bool closeRequest;
    bool handShakeDone;
    bool insidePersistent;
    bool persistentRequest = false;

public slots:
    void onHandshakeTimeout();
    void onTimeout();
    void sendPersistant(Packet sendpacket);
    void handShakeComplited();
    void receivedDatagram(QByteArray plainText);

signals:
    void toStatusBar(const QString & message, int timeout = 0, bool override = false);
    void connectStatus(QString);
    void packetSent(Packet);
    void packetReceived(Packet);


};
#else

class Dtlsthread : public QThread
{
    Q_OBJECT

public:
    Dtlsthread(Packet sendPacket, QObject *parent) ;
    virtual ~Dtlsthread();
    void run() override; // Pure virtual function making this class abstract


    QTimer* timer;
    std::vector<DtlsAssociation*> dtlsAssociations;
    DtlsAssociation* dtlsAssociation;
    QString recievedMassage;
    Packet sendpacket;


    bool closeRequest;
    bool handShakeDone;
    bool insidePersistent;
    bool persistentRequest = false;

};

#endif

