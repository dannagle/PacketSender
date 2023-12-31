#ifndef WAKEONLAN_H
#define WAKEONLAN_H

#include <QDialog>

#include "packet.h"

namespace Ui {
class WakeOnLAN;
}

class WakeOnLAN : public QDialog
{
    Q_OBJECT

public:
    explicit WakeOnLAN(QWidget *parent = nullptr);
    ~WakeOnLAN();
    void setTarget(QString mac, int portIndex);
    Packet generatedPacket;
    QString mac;
    int portIndex;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::WakeOnLAN *ui;
};

#endif // WAKEONLAN_H
