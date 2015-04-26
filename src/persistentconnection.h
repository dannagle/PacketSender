#ifndef PERSISTENTCONNECTION_H
#define PERSISTENTCONNECTION_H

#include <QDialog>
#include <QTimer>
#include "packet.h"
#include "tcpthread.h"

namespace Ui {
class PersistentConnection;
}

class PersistentConnection : public QDialog
{
    Q_OBJECT

public:
    explicit PersistentConnection(QWidget *parent = 0);
    ~PersistentConnection();
    Packet sendPacket;

    void init();
public slots:
    void packetToSend(Packet sendpacket);
    void refreshTimerTimeout();
    void aboutToClose();
private slots:
    void on_buttonBox_rejected();

private:
    Ui::PersistentConnection *ui;
    TCPThread *thread;
    QTimer refreshTimer;

};

#endif // PERSISTENTCONNECTION_H
