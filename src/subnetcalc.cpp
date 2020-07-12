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


    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface eth;

    setWindowTitle("IPv4 Subnet Calculator");

    QString startLog = "Your non-loopback addresses: \n\n";
    QTextStream out(&startLog);


    QNetworkAddressEntry entry;

    QList<QNetworkAddressEntry> allEntries = SubnetCalc::nonLoopBackAddresses();
    foreach (entry, allEntries) {
        out << entry.ip().toString() << "  /  " << entry.netmask().toString() << "\n";
    }


    ui->resultEdit->setText(startLog);

}


QList<QNetworkAddressEntry> SubnetCalc::nonLoopBackAddresses()
{

    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface eth;

    QList<QNetworkAddressEntry> allEntriesNonLoopback;
    allEntriesNonLoopback.clear();

    foreach (eth, allInterfaces) {
        QList<QNetworkAddressEntry> allEntries = eth.addressEntries();
        if (allEntries.size() == 0 || !eth.flags().testFlag(QNetworkInterface::IsUp)) {
            continue;
        }

        QString ethString;
        QTextStream ethOut(&ethString);


        ethOut << "\nFor " << eth.humanReadableName() << " (" << eth.hardwareAddress() << ")" << ":\n";
        QNetworkAddressEntry entry;


        foreach (entry, allEntries) {
            if (!entry.ip().isLoopback()) {
                if (entry.ip().toString().contains(":")) {
                    //IPv6 to the end
                    allEntriesNonLoopback.append(entry);
                } else {
                    //IPv4 to the front
                    allEntriesNonLoopback.prepend(entry);
                }
            }
        }
    }

    return allEntriesNonLoopback;


}



void SubnetCalc::populate()
{

    QString ip = ui->ipEdit->text().trimmed();
    QString subnet = ui->subnetEdit->text().trimmed();

    //remove forward slash if added
    subnet = subnet.replace("/", "");

    if (subnet.contains(":")) { //ipv6 subnet
        QStringList colons = subnet.split(":");
        QString colon;
        int counter = 0;
        bool valid = true;
        foreach (colon, colons) {
            bool ok;
            unsigned int hex = colon.toUInt(&ok, 16);
            if (ok) {
                QString bits(QString::number(hex, 2));
                int ones = bits.count("1");
                if (bits.contains("01")) {
                    QDEBUG() << "Invalid netmask:" << bits;
                    valid = false;
                    break;
                } else {
                    counter += ones;
                }
            }
        }

        if (valid) {
            subnet = QString::number(counter);
        }

    }


    QPair<QHostAddress, int> testAddress = QHostAddress::parseSubnet(ip + "/" + subnet);

    //private ipv4 addresses..
    //10.0.0.0/8
    //172.16.0.0/12
    //192.168.0.0/16


    if (!testAddress.first.isNull()) {
        QDEBUG() << "Valid address:" << testAddress.first.toString() << "/" << testAddress.second;

        //do the calculations...

        bool isIPv6 = testAddress.first.toString().contains(":");
        bool isIPv4 = !isIPv6;

        if (isIPv4) {
            QHostAddress ipv4Address;
            QDEBUG() << "IPv4 detected";
            quint32 ipv4 = testAddress.first.toIPv4Address();
            int subnetBits = testAddress.second;
            int numAddresses = 1 << (32 - subnetBits);
            QDEBUGVAR(numAddresses);
            quint32 subnet = 0;

            for (int i = 0; i < subnetBits; i++) {
                subnet = (subnet << 1) + 1;
            }

            for (int i = subnetBits; i < 32; i++) {
                subnet = (subnet << 1);
            }

            quint32 startSubnet = (ipv4 & subnet);
            quint32 firstAddress = startSubnet + 1;
            quint32 lastAddress = startSubnet + numAddresses - 2;
            quint32 broadcastAddress = startSubnet + numAddresses - 1;
            ipv4Address.setAddress(firstAddress);
            ui->startEdit->setText(ipv4Address.toString());
            ipv4Address.setAddress(lastAddress);
            ui->endEdit->setText(ipv4Address.toString());
            ipv4Address.setAddress(broadcastAddress);
            ui->broadcastEdit->setText(ipv4Address.toString());

        }
        if (isIPv6) {
            QDEBUG() << "IPv6 detected";
        }
        //QDEBUGVAR();



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
    ui->broadcastEdit->clear();
    ui->ipEdit->clear();
    ui->subnetEdit->clear();
    ui->ipsubnetCheckEdit->clear();
    ui->startEdit->clear();
    ui->endEdit->clear();
    ui->broadcastEdit->clear();
    ui->checkSubnetResultEdit->clear();
}

void SubnetCalc::on_ipsubnetCheckEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface eth;

    QString testIP = ui->ipsubnetCheckEdit->text().trimmed();
    QDEBUGVAR(testIP);


    //is this address valid?
    QPair<QHostAddress, int> testAddress = QHostAddress::parseSubnet(testIP + "/24");
    if (testAddress.first.toString().isEmpty()) {
        ui->checkSubnetResultEdit->clear();
        return;
    }
    QDEBUGVAR(testAddress.first.toString());


    //It is good enough to start testing...

    bool found = false;
    foreach (eth, allInterfaces) {
        QList<QNetworkAddressEntry> allEntries = eth.addressEntries();
        QNetworkAddressEntry entry;
        if (allEntries.size() == 0) {
            continue;
        }

        foreach (entry, allEntries) {
            QHostAddress entryAddress = entry.ip();
            if (entryAddress.toString().isEmpty()) {
                continue;
            }

            testAddress = QHostAddress::parseSubnet(testIP + "/" + entry.netmask().toString());
            if (testAddress.first.toString().isEmpty()) {
                continue;
            }


            found = entryAddress.isInSubnet(testAddress);


            QDEBUG() << found << entryAddress.toString() << "->" << testAddress.first.toString() << "/" << testAddress.second;


            if (found) {
                QString resultText = "Within ";
                resultText.append(entry.ip().toString());
                resultText.append(" / ");
                resultText.append(entry.netmask().toString());
                ui->checkSubnetResultEdit->setText(resultText);
                return;
            }
        }

        if (!found) {
            ui->checkSubnetResultEdit->setText("No entries in subnet.");
        }
    }



}
