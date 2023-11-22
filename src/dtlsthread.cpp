#include "dtlsthread.h"
#include "packet.h"

Dtlsthread::Dtlsthread(Packet sendPacket, QObject *parent)
    : QThread(parent), sendPacket(sendPacket)
{}

Dtlsthread::~Dtlsthread() {
    // Destructor implementation (can be empty for this example)
}


void Dtlsthread::run()
{
    // Implement the run function
    // If run() is meant to be pure virtual, then this implementation should be in a subclass.
}











//#include "dtlsthread.h"
//#include <QDebug>



//Dtlsthread::Dtlsthread(Packet sendPacket, QObject *parent)
//    : QThread(parent), sendPacket(sendPacket)
//{


//}

//void Dtlsthread::run() {
//}


