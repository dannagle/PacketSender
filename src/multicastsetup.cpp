#include "multicastsetup.h"
#include "ui_multicastsetup.h"

MulticastSetup::MulticastSetup(PacketNetwork *pNetwork, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MulticastSetup)
{
    this->packetNetwork = pNetwork;
    ui->setupUi(this);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle("IPv4 Multicast Setup");

    init();
}

void MulticastSetup::init()
{

    QStringList mcastStringList = packetNetwork->multicastStringList();
    QString mcast, ip, port;

    QStringList tHeaders, split;

    tHeaders << "Address" << "Port";

    ui->mcastTable->setColumnCount(tHeaders.size());
    ui->mcastTable->verticalHeader()->show();
    ui->mcastTable->horizontalHeader()->show();
    ui->mcastTable->setHorizontalHeaderLabels(tHeaders);


    ui->mcastTable->setRowCount(mcastStringList.size());

    QDEBUGVAR(mcastStringList.size());
    for(int i=0; i < mcastStringList.size(); i++) {
        mcast = mcastStringList.at(i);

        split = mcast.split(":");
        if(split.size() < 2) continue;

        QTableWidgetItem * tItem0 = new QTableWidgetItem(split[0]);
        QTableWidgetItem * tItem1 = new QTableWidgetItem(split[1]);

        ui->mcastTable->setItem(i, 0, tItem0);
        ui->mcastTable->setItem(i, 1, tItem1);
    }



    ui->mcastTable->setSortingEnabled(true);
    ui->mcastTable->resizeColumnsToContents();
    ui->mcastTable->resizeRowsToContents();
    ui->mcastTable->horizontalHeader()->setStretchLastSection(true);


}

MulticastSetup::~MulticastSetup()
{
    delete ui;
}

void MulticastSetup::on_joinButton_clicked()
{

    QString ip = ui->ipaddressEdit->text();
    int port = ui->portEdit->text().toInt();

    if(port > 0) {
        packetNetwork->joinMulticast(ip, port);
    }
    QDEBUG();
    init();
}
