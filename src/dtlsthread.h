#ifndef BASETHREAD_H
#define BASETHREAD_H

#include <QThread>
#include "packet.h"
#include "QSettings"
#include "association.h"
//#include "QTimer"

class Dtlsthread : public QThread
{
    Q_OBJECT

public:
    Dtlsthread(Packet sendPacket, QObject *parent);
    DtlsAssociation* initDtlsAssociation();
    virtual ~Dtlsthread();
    void persistentConnectionLoop();
    void run() override; // Pure virtual function making this class abstract
    std::vector<QString> getCmdInput(Packet sendpacket, QSettings& settings);
    void writeMassage(Packet packetToSend, DtlsAssociation* dtlsAssociation);

    QTimer* timer;
    std::vector<DtlsAssociation*> dtlsAssociations;
    DtlsAssociation* dtlsAssociation;
    QString recievedMassage;
    Packet sendpacket;


    bool closeRequest;
    bool handShakeDone;
    bool insidePersistent;

public slots:
    void onTimeout();
    void sendPersistant(Packet sendpacket);
    void handShakeComplited();
    void receivedDatagram(QByteArray plainText);

signals:
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

