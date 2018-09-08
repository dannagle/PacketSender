#include "multicastsetup.h"
#include "ui_multicastsetup.h"

#include <QMessageBox>

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

    QString ip = ui->ipaddressEdit->text().trimmed();
    int port = ui->portEdit->text().toInt();

    if (!PacketNetwork::isMulticast(ip)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Not Multicast.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("IP must be a multicast address.");
        msgBox.exec();
        ui->ipaddressEdit->setFocus();
        ui->ipaddressEdit->selectAll();
        return;
    }

    if(port < 1) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Bad Port.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Bind port must be a number greater than 0.");
        msgBox.exec();
        ui->portEdit->setFocus();
        ui->portEdit->selectAll();
        return;

    }

    QDEBUG();
    packetNetwork->joinMulticast(ip, port);
    QDEBUG();
    init();
}
