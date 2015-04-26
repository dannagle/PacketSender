#include "persistentconnection.h"
#include "ui_persistentconnection.h"

PersistentConnection::PersistentConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersistentConnection)
{
    ui->setupUi(this);
    sendPacket.clear();
}

PersistentConnection::~PersistentConnection()
{
    delete ui;
}
