#include "wakeonlan.h"
#include "ui_wakeonlan.h"

#include "globals.h"
#include <QDebug>

WakeOnLAN::WakeOnLAN(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WakeOnLAN)
{
    ui->setupUi(this);
}

WakeOnLAN::~WakeOnLAN()
{
    delete ui;
}

void WakeOnLAN::on_buttonBox_accepted()
{
    generatedPacket = Packet::generateWakeOnLAN(ui->macEdit->text(), ui->portBox->currentText().toUInt());

}

