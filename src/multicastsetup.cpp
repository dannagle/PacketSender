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

void MulticastSetup::setIPandPort(QString ip, unsigned int port)
{
    ui->ipaddressEdit->setText(ip);
    ui->portEdit->setText(QString::number(port));
    ui->joinButton->setFocus();
}

void MulticastSetup::init()
{

    QStringList mcastStringList = packetNetwork->multicastStringList();
    QString mcast, ip;
    unsigned int port;

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

        PacketNetwork::multiCastToIPandPort(mcast, ip, port);

        ui->mcastTable->setItem(i, 0, new QTableWidgetItem(ip));
        ui->mcastTable->setItem(i, 1, new QTableWidgetItem(QString::number(port)));
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
        msgBox.setText("IP must be an IPv4 multicast address.\n(224.0.0.0 to 239.255.255.255)");
        msgBox.exec();
        ui->ipaddressEdit->setFocus();
        ui->ipaddressEdit->selectAll();
        return;
    }

    if((port < 1) || port > 65535) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Invalid Port.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Bind port must be a number 1-65535.");
        msgBox.exec();
        ui->portEdit->setFocus();
        ui->portEdit->selectAll();
        return;

    }

    int ipMode = packetNetwork->getIPmode();

    if(ipMode != 4) {

        QMessageBox msgBox;
        msgBox.setWindowTitle("IPv4-only.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Packet Sender supports multicast when binded to IPv4 only. \nTurn off IPv6 support and switch to IPv4-only?");
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::No) {
            return;
        }
        packetNetwork->setIPmode(4);
        packetNetwork->kill();
        packetNetwork->init();

    }


    QDEBUG();
    packetNetwork->joinMulticast(ip, port);
    QDEBUG();
    init();
}

void MulticastSetup::on_leaveButton_clicked()
{

    QDEBUG();
    packetNetwork->leaveMulticast();
    init();
}
