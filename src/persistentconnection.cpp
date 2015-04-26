#include "persistentconnection.h"
#include "ui_persistentconnection.h"

PersistentConnection::PersistentConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersistentConnection)
{
    ui->setupUi(this);
    QDEBUG();
    sendPacket.clear();
    QDEBUG() << ": refreshTimer Connection attempt " <<
                connect ( &refreshTimer , SIGNAL ( timeout() ) , this, SLOT ( refreshTimerTimeout (  ) ) )
             << connect ( this , SIGNAL ( rejected() ) , this, SLOT ( aboutToClose (  ) ) )
             << connect ( this , SIGNAL ( accepted() ) , this, SLOT ( aboutToClose (  ) ) )
             << connect ( this , SIGNAL ( dialogIsClosing() ) , this, SLOT ( aboutToClose (  ) ) );
    QDEBUG() << "Setup timer";
    refreshTimer.setInterval(2000);
    QDEBUG() << "Starting timer";
    refreshTimer.start();
    QDEBUG() << "Finished";


}

void PersistentConnection::aboutToClose() {
    QDEBUG() << "Stopping timer";
    refreshTimer.stop();
    QDEBUG() << "Stopping thread";
    thread->quit();

}


PersistentConnection::~PersistentConnection()
{
    aboutToClose();
    delete ui;
}


/*
void PersistentConnection::reject()
{
    QDEBUG() << "Stopping timer";
    refreshTimer.stop();

}
*/

void PersistentConnection::init() {

    QDEBUG();
    setWindowTitle("TCP://"+sendPacket.toIP + ":" + QString::number(sendPacket.port));
    QDEBUG();

    thread = new TCPThread(sendPacket, this);

    QApplication::processEvents();

    thread->start();


    QApplication::processEvents();

}

void PersistentConnection::packetToSend(Packet sendpacket)
{
    QDEBUG();

}

void PersistentConnection::refreshTimerTimeout() {
    QDEBUG();

}

void PersistentConnection::on_buttonBox_rejected()
{

    QDEBUG() << "Stopping timer";
    refreshTimer.stop();

}
