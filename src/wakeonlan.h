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
    Packet generatedPacket;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::WakeOnLAN *ui;
};

#endif // WAKEONLAN_H
