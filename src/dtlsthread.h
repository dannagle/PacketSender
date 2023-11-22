#ifndef BASETHREAD_H
#define BASETHREAD_H

#include <QThread>
#include "packet.h"

class Dtlsthread : public QThread
{
    Q_OBJECT


public:
    Dtlsthread(Packet sendPacket, QObject *parent);
    virtual ~Dtlsthread();
    Packet sendPacket;
    void run() override; // Pure virtual function making this class abstract
    void sendPersistant();



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

