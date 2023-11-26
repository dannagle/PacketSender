#ifndef BASETHREAD_H
#define BASETHREAD_H

#include <QThread>
#include "packet.h"
#include "QSettings"
#include "association.h"

class Dtlsthread : public QThread
{
    Q_OBJECT


public:
    Dtlsthread(Packet sendPacket, QObject *parent);
    virtual ~Dtlsthread();
    std::vector<DtlsAssociation*> dtlsAssociations;

    Packet sendpacket;
    void run() override; // Pure virtual function making this class abstract
    void sendPersistant();
    std::vector<QString> getCmdInput(Packet sendpacket, QSettings& settings);
public slots:
    void writeMassage(Packet packetToSend, DtlsAssociation* dtlsAssociation);
    void addServerResponse(const QString &clientInfo, const QByteArray &datagram, const QByteArray &plainText, QHostAddress serverAddress, quint16 serverPort, quint16 userPort);




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

