#ifndef BASETHREAD_H
#define BASETHREAD_H

#include <QThread>
#include "packet.h"
#include "globals.h"
#include "QSettings"
#include "association.h"
//#include "QTimer"

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

#endif // BASETHREAD_H




//#ifndef DTLSTHREAD_H
//#define DTLSTHREAD_H
//#include "packet.h"
//#include <QThread>

//class Dtlsthread : public QThread
//{
//    Q_OBJECT

//public:
//    Packet sendPacket;


//    Dtlsthread(Packet sendPacket, QObject *parent);
//    void run() override; // Entry point for the thread
////    void setParameters(int param); // Setter for parameters

////private:
////    int m_param; // Parameter to be used in the thread
//};
//#endif

