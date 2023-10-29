#include "wakeonlan.h"
#include "ui_wakeonlan.h"

#include "globals.h"
#include <QDebug>

WakeOnLAN::WakeOnLAN(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WakeOnLAN)
{
    ui->setupUi(this);

    mac.clear();
    portIndex = 0;


}


WakeOnLAN::~WakeOnLAN()
{
    delete ui;
}

void WakeOnLAN::setTarget(QString mac, int portIndex)
{
    ui->macEdit->setText(mac);
    ui->portBox->setCurrentIndex(portIndex);

}

void WakeOnLAN::on_buttonBox_accepted()
{
    mac = ui->macEdit->text().trimmed().toUpper();
    portIndex = ui->portBox->currentIndex();
    generatedPacket = Packet::generateWakeOnLAN(mac, ui->portBox->currentText().toUInt());

}

