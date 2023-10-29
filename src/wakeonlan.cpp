#include "wakeonlan.h"
#include "ui_wakeonlan.h"

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
