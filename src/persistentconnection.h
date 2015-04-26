#ifndef PERSISTENTCONNECTION_H
#define PERSISTENTCONNECTION_H

#include <QDialog>
#include <QTimer>
#include <QList>

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

signals:
    void persistantPacketSend(Packet sendpacket);


public slots:
    void packetToSend(Packet sendpacket);
    void refreshTimerTimeout();
    void aboutToClose();
    void statusReceiver(QString message);

    void packetSentSlot(Packet pkt);
private slots:
    void on_buttonBox_rejected();

    void on_asciiSendButton_clicked();

private:
    Ui::PersistentConnection *ui;
    TCPThread *thread;
    QTimer refreshTimer;
    QList<Packet> trafficList;

};

#endif // PERSISTENTCONNECTION_H
