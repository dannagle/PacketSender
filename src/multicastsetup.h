#ifndef MULTICASTSETUP_H
#define MULTICASTSETUP_H

#include <QDialog>
#include <QDebug>
#include "packetnetwork.h"
#include "globals.h"


namespace Ui {
class MulticastSetup;
}

class MulticastSetup : public QDialog
{
    Q_OBJECT

public:
    explicit MulticastSetup(PacketNetwork * pNetwork, QWidget *parent = nullptr);
    void setIPandPort(QString ip, unsigned int port);
    ~MulticastSetup();

    void init();
private slots:
    void on_joinButton_clicked();

    void on_leaveButton_clicked();

private:
    Ui::MulticastSetup *ui;
    PacketNetwork * packetNetwork;
};

#endif // MULTICASTSETUP_H
