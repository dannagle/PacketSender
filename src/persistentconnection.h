#ifndef PERSISTENTCONNECTION_H
#define PERSISTENTCONNECTION_H

#include <QDialog>
#include "packet.h"

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

public slots:
    void packetToSend(Packet sendpacket);

private:
    Ui::PersistentConnection *ui;
};

#endif // PERSISTENTCONNECTION_H
