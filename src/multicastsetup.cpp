#include "multicastsetup.h"
#include "ui_multicastsetup.h"

MulticastSetup::MulticastSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MulticastSetup)
{
    ui->setupUi(this);
}

MulticastSetup::~MulticastSetup()
{
    delete ui;
}
