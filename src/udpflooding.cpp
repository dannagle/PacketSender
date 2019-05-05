#include "udpflooding.h"
#include "ui_udpflooding.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QList>
#include <QDebug>
#include <QNetworkAddressEntry>
#include "globals.h"


UDPFlooding::UDPFlooding(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UDPFlooding)
{
    ui->setupUi(this);


    setWindowTitle("UDP Traffic Flooding");

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    thread = new ThreadSender(this);
    thread->ip = "192.168.1.1";
    thread->port = 5000;
    thread->rate = 1000;
    thread->ascii = "Hello World";

    ui->ipEdit->setText(thread->ip);
    ui->portEdit->setText(QString::number(thread->port));
    ui->rateEdit->setText(QString::number(thread->rate));
    ui->asciiEdit->setText(thread->ascii);

    thread->issending = false;
    thread->stopsending = false;


    ui->startButton->setDisabled(true);
    ui->stopButton->setDisabled(true);

    //logging
    refreshTimer.setInterval(500);
    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout())) ;
    refreshTimer.start();



}

UDPFlooding::~UDPFlooding()
{
    delete ui;
}

void UDPFlooding::on_startButton_clicked()
{
    QDEBUG();

    bool ok1, ok2;

    thread->port = ui->portEdit->text().toUInt(&ok1);
    thread->rate = ui->rateEdit->text().toULong(&ok2);
    thread->ascii = ui->asciiEdit->text();
    thread->ip = ui->ipEdit->text();

    if(!ok1) {
        ui->portEdit->setText("Invalid");
        return;
    }

    if(!ok2) {
        ui->rateEdit->setText("Invalid");
        return;
    }

    // Do it.
    thread->start();


}



void UDPFlooding::on_stopButton_clicked()
{
    QDEBUG() << "Flagging stop send";

    thread->stopsending = true;
}

void UDPFlooding::refreshTimerTimeout()
{
    QDEBUGVAR(thread->issending);

    if(!thread->issending) {
        ui->startButton->setDisabled(false);
        ui->stopButton->setDisabled(true);
    } else {
        ui->startButton->setDisabled(true);
        ui->stopButton->setDisabled(false);
    }

}


ThreadSender::ThreadSender(QObject *parent) : QThread(parent)
{
    QDEBUG();
}


void ThreadSender::run()
{
    QDEBUG() << "Begin send";

    issending = true;
    stopsending = false;
    starttime = QDateTime::currentDateTime();
    packetssent = 0;


    while(!stopsending) {
        //intense send
        msleep(1); //momentarily break thread
        //QDEBUG() << "Sending...";
    }

    stopsending = false;
    issending = false;

    QDEBUG() << "End send";

}
