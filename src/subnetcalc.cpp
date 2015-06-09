#include "subnetcalc.h"
#include "ui_subnetcalc.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QList>
#include <QDebug>
#include <QNetworkAddressEntry>
#include "globals.h"


SubnetCalc::SubnetCalc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubnetCalc)
{
    ui->setupUi(this);

    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface eth;

    setWindowTitle("Subnet Calculator");

    QString startLog = "IP addresses found: \n";
    QTextStream out (&startLog);

    foreach(eth, allInterfaces) {
        QList<QNetworkAddressEntry> allEntries = eth.addressEntries();
        if(allEntries.size() == 0) {
            continue;
        }
        QString ethString;
        QTextStream ethOut (&ethString);


        ethOut << "\nFor " << eth.name() << ":\n";
        QNetworkAddressEntry entry;

        int nonLoopBack = 0;

        foreach (entry, allEntries) {
            if(!entry.ip().isLoopback()) {
                nonLoopBack = 1;
                ethOut << entry.ip().toString() << "  /  " << entry.netmask().toString() << "\n";
            }
        }

        if(nonLoopBack) {
            out << ethString;
        }
    }

    ui->resultEdit->setText(startLog);




}


void SubnetCalc::populate() {

    QString ip = ui->ipEdit->text().trimmed();
    QString subnet = ui->subnetEdit->text().trimmed();

    //remove forward slash if added
    subnet = subnet.replace("/", "");


    QPair<QHostAddress, int> testAddress = QHostAddress::parseSubnet(ip + "/" + subnet);



    if(!testAddress.first.isNull()) {
        QDEBUG() << "Valid address:" << testAddress.first.toString() << "/" << testAddress.second;

        //do the calculations...



    } else {
        QDEBUG() << "Invalid";

    }

}

SubnetCalc::~SubnetCalc()
{
    delete ui;
}

void SubnetCalc::on_ipEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    QDEBUG();
    populate();
}

void SubnetCalc::on_subnetEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    QDEBUG();
    populate();
}

void SubnetCalc::on_clearButton_clicked()
{
    ui->resultEdit->clear();
}
