/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QDesktopServices>
#include <QScrollBar>
#include <QFileDialog>
#include <QLibrary>
#include <QFile>
#include <QTime>
#include <QHostAddress>
#include <QHostInfo>
#include <QShortcut>
#include <QClipboard>
#include "brucethepoodle.h"


int hexToInt(QChar hex);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timeSinceLaunch();


#ifndef CONSOLE_BUILD
#ifdef Q_OS_WINDOWS


    QStringList args = QApplication::arguments();
    if(args.size() > 0) {

        QMessageBox msgBox;
        msgBox.setWindowTitle("Oops! You launched the GUI!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The \".com\" should only be used for command line Packet Sender");
        msgBox.exec();

    }

#endif
#endif
    //create temp folders if not exist
    QDir mdir;
    mdir.mkpath(TEMPPATH);
    mdir.mkpath(SETTINGSPATH);

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);


    ui->mainTabWidget->setCurrentIndex(0);
    ui->settingsTabWidget->setCurrentIndex(0);

    ui->buttonScrollArea->setMaximumHeight(60);

    QDate vDate = QDate::fromString(QString(__DATE__).simplified(), "MMM d yyyy");


    QDEBUGVAR(__DATE__);
    QDEBUGVAR(vDate.toString("yyyy-MM-dd"));

    ui->buidDateLabel->setText("Build date: " + QString(__DATE__));

    ui->buidDateLabel->setText("Build date: " + vDate.toString("yyyy-MM-dd"));


    //TODO not working yet
    ui->persistentConnectCheck->show();

    hyperLinkStyle = "QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ";

    QIcon mIcon(":pslogo.png");

    ui->displayOrderList->clear();
    ui->displayOrderListTraffic->clear();
    setDefaultTableHeaders();
    setStoredTableHeaders();
    loadTableHeaders();


    sendButtonHolder.clear();

    lastSendPacket.clear();



    setWindowTitle("Packet Sender");
    setWindowIcon(mIcon);

    ui->menuBar->hide();

    tableActive = false;


    maxLogSize = 0;

    ui->udpServerEnableCheck->setChecked(settings.value("udpServerEnable", true).toBool());
    ui->tcpServerEnableCheck->setChecked(settings.value("tcpServerEnable", true).toBool());
    ui->attemptReceiveCheck->setChecked(settings.value("attemptReceiveCheck", false).toBool());

    ui->delayAfterConnectCheck->setChecked(settings.value("delayAfterConnectCheck", false).toBool());
    ui->persistentConnectCheck->setChecked(settings.value("persistentConnectCheck", false).toBool());


    ui->hideQuickSendCheck->setChecked(settings.value("hideQuickSendCheck", false).toBool());


    if(ui->hideQuickSendCheck->isChecked()) {
        ui->buttonScrollArea->hide();
        ui->selectedButtonsLabel->hide();
    } else {
        ui->buttonScrollArea->show();
        ui->selectedButtonsLabel->show();
    }

    ui->rolling500entryCheck->setChecked(settings.value("rolling500entryCheck", false).toBool());
    ui->copyUnformattedCheck->setChecked(settings.value("copyUnformattedCheck", false).toBool());


    if(ui->rolling500entryCheck->isChecked()) {
        maxLogSize = 100;
    }


    ui->sendResponseSettingsCheck->setChecked(settings.value("sendReponse", false).toBool());

    ui->hexResponseEdit->setText(settings.value("responseHex","").toString());
    on_hexResponseEdit_editingFinished();


    http = new QNetworkAccessManager(this); //the only http object (required by Qt)

    QDEBUG() << " http connect attempt:" << connect(http, SIGNAL(finished(QNetworkReply*)),
         this, SLOT(httpFinished(QNetworkReply*)));
    //http->get(QNetworkRequest(QUrl("http://packetsender.com/")));


    QDEBUG() << " packet send connect attempt:" << connect(this, SIGNAL(sendPacket(Packet)),
         &packetNetwork, SLOT(packetToSend(Packet)));

    packetNetwork.init();

    packetNetwork.responseData = ui->hexResponseEdit->text().trimmed();
    packetNetwork.sendResponse = ui->sendResponseSettingsCheck->isChecked();


    //http->get(QNetworkRequest(QUrl("http://packetsender.com/")));

    //Connect statusbar to packetNetwork
    QDEBUG() << ": packetNetwork -> Statusbar connection attempt" <<
               connect(&packetNetwork, SIGNAL(toStatusBar(const QString &, int, bool)),
                       this, SLOT(statusBarMessage(const QString &, int, bool)));

    //Connect packetNetwork to trafficlog
    QDEBUG() << ": packetSent -> toTrafficLog connection attempt" <<
               connect(&packetNetwork, SIGNAL(packetSent(Packet)),
                       this, SLOT(toTrafficLog(Packet)));


    packetsSaved = Packet::fetchAllfromDB("");
    loadPacketsTable();

 //   statusBar()->insertPermanentWidget(0, generatePSLink());
 //   statusBar()->insertPermanentWidget(1, generateDNLink());



    stopResendingButton = new QPushButton("Resending");
    stopResendingButton->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    stopResendingButton->setFlat(true);
    stopResendingButton->setCursor(Qt::PointingHandCursor);
    stopResendingButton->setIcon(QIcon(PSLOGO));

    connect(stopResendingButton, SIGNAL(clicked()),
            this, SLOT(cancelResends()));


    statusBar()->insertPermanentWidget(1, stopResendingButton);

    stopResendingButton->hide();

    udpServerStatus = new QPushButton("UDP:"+QString::number(packetNetwork.getUDPPort()));
    udpServerStatus->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    udpServerStatus->setFlat(true);
    udpServerStatus->setCursor(Qt::PointingHandCursor);
    udpServerStatus->setIcon(QIcon(UDPRXICON));

    connect(udpServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleUDPServer()));


    statusBar()->insertPermanentWidget(2, udpServerStatus);


    //updatewidget
    tcpServerStatus = new QPushButton("TCP:"+QString::number(packetNetwork.getTCPPort()));
    tcpServerStatus->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    tcpServerStatus->setFlat(true);
    tcpServerStatus->setCursor(Qt::PointingHandCursor);
    tcpServerStatus->setIcon(QIcon(TCPRXICON));


    connect(tcpServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleTCPServer()));


    statusBar()->insertPermanentWidget(3, tcpServerStatus);


    UDPServerStatus();
    TCPServerStatus();


    refreshTimer.setInterval(500);
    qDebug() << __FILE__ << "/" <<__LINE__  << ": refreshTimer Connection attempt " <<
                connect ( &refreshTimer , SIGNAL ( timeout() ) , this, SLOT ( refreshTimerTimeout (  ) ) ) ;

    packetsLogged.clear();
    loadTrafficLogTable();

    ui->DNLinkButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->DNLinkButton->setFlat(true);
    ui->DNLinkButton->setCursor(Qt::PointingHandCursor);
    connect(ui->DNLinkButton, SIGNAL(clicked()),
            this, SLOT(gotoDanNagleDotCom()));


    ui->twitterButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->twitterButton->setIcon( QIcon(":Twitter_logo_blue.png"));
    ui->twitterButton->setFlat(true);
    ui->twitterButton->setCursor(Qt::PointingHandCursor);
    connect(ui->twitterButton, SIGNAL(clicked()),
            this, SLOT(gotoNagleCode()));


    ui->DNAmazonLinkButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->DNAmazonLinkButton->setIcon( QIcon(":pslogo.png"));
    ui->DNAmazonLinkButton->setFlat(true);
    ui->DNAmazonLinkButton->setCursor(Qt::PointingHandCursor);
    connect(ui->DNAmazonLinkButton, SIGNAL(clicked()),
            this, SLOT(gotoDanNaglePayPal()));


    ui->psLinkButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->psLinkButton->setIcon( QIcon(":pslogo.png"));
    ui->psLinkButton->setFlat(true);
    ui->psLinkButton->setCursor(Qt::PointingHandCursor);
    connect(ui->psLinkButton, SIGNAL(clicked()),
            this, SLOT(gotoPacketSenderDotCom()));


    refreshTimer.start();



        //Bruce is my pet poodle.
        //Dog easter egg.  CTRL D, O, G.
        //             or  CMD D, O, G.
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D,Qt::CTRL + Qt::Key_O, Qt::CTRL + Qt::Key_G ), this);
    QDEBUG() << ": dog easter egg Connection attempt " << connect(shortcut, SIGNAL(activated()), this, SLOT(poodlepic()));


    QDEBUG() << "Load time:" <<  timeSinceLaunch();

    QDEBUG() << "Settings file loaded" << SETTINGSFILE;
    QDEBUG() << "Packets file loaded" << PACKETSFILE;

}

QPushButton * MainWindow::generatePSLink()
{
    QPushButton * returnButton = new QPushButton("PacketSender.com");
    returnButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    returnButton->setIcon( QIcon(":pslogo.png"));
    returnButton->setFlat(true);
    returnButton->setCursor(Qt::PointingHandCursor);
    connect(returnButton, SIGNAL(clicked()),
            this, SLOT(gotoPacketSenderDotCom()));

    return returnButton;

}

void MainWindow::toTrafficLog(Packet logPacket)
{

    QDEBUG() << logPacket.name;

    if(ui->logTrafficCheck->isChecked())
    {
        packetsLogged.append(logPacket);
    }

}


void MainWindow::UDPServerStatus()
{
    if(packetNetwork.getUDPPort() > 0)
    {
        udpServerStatus->setText("UDP:"+QString::number(packetNetwork.getUDPPort()));
        ui->udpServerPortEdit->setText(QString::number(packetNetwork.getUDPPort()));

    } else {
        udpServerStatus->setText("UDP Server Disabled");

    }

    //updatewidget


}

void MainWindow::TCPServerStatus()
{
    if(packetNetwork.getTCPPort() > 0)
    {
        tcpServerStatus->setText("TCP:"+QString::number(packetNetwork.getTCPPort()));
        ui->tcpServerPortEdit->setText(QString::number(packetNetwork.getTCPPort()));
    } else {
        tcpServerStatus->setText("TCP Server Disabled");

    }


}

QPushButton *MainWindow::generateDNLink()
{

    QPushButton * returnButton = new QPushButton("DanNagle.com");
    returnButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    returnButton->setIcon( QIcon(":dannagle_logo.png"));
    returnButton->setFlat(true);
    returnButton->setCursor(Qt::PointingHandCursor);
    connect(returnButton, SIGNAL(clicked()),
            this, SLOT(gotoDanNagleDotCom()));

    return returnButton;

}

void MainWindow::loadTrafficLogTable()
{

    QTableWidgetItem * tItem;
    Packet tempPacket;

    QListWidget * lw = ui->displayOrderListTraffic;
    QListWidgetItem * item;
    QStringList lwStringList;
    lwStringList.clear();
    QString text;
    int size = lw->count();

    QDEBUGVAR(size);
    for(int i = 0; i < size; i++)
    {
        item = lw->item(i);
        text = item->text();
        lwStringList.append(text);
    }


    ui->trafficLogTable->setColumnCount(lwStringList.size());

    ui->trafficLogTable->verticalHeader()->show();
    ui->trafficLogTable->horizontalHeader()->show();
    ui->trafficLogTable->setHorizontalHeaderLabels(lwStringList);


    QList<Packet> displayPackets;
    displayPackets.clear();


    QString filterString = ui->searchTrafficEdit->text().toLower().trimmed();

    if(filterString.isEmpty())
    {
        displayPackets = packetsLogged;
    } else {
        foreach(tempPacket, packetsLogged)
        {

            if(tempPacket.name.toLower().contains(filterString) ||
                    tempPacket.toIP.contains(filterString) ||
                    tempPacket.hexToASCII(tempPacket.hexString).toLower().contains(filterString) ||
                    tempPacket.hexString.toLower().contains(filterString)
                    )
            {
                displayPackets.append(tempPacket);
                continue;
            }
        }

    }




    Packet::sortByTime(displayPackets);

    if(displayPackets.isEmpty())
    {
        ui->trafficLogTable->setRowCount(0);
    } else {
        ui->trafficLogTable->setRowCount(displayPackets.count());
    }
    unsigned int rowCounter = 0;
    unsigned int colCount = 0;

    QDEBUGVAR(lwStringList);

    ui->trafficLogTable->setSortingEnabled(false);

    foreach(tempPacket, displayPackets)
    {

        colCount = 0;
/*

lwStringList ("Time", "From IP", "From Port", "To IP", "To Port", "Method", "Error", "ASCII", "Hex")

*/

        //
        tItem = new QTableWidgetItem(tempPacket.timestamp.toString(DATETIMEFORMAT));
        tItem->setIcon(tempPacket.getIcon());
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "Time"), tItem);

        tItem = new QTableWidgetItem(tempPacket.fromIP);
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "From IP"), tItem);

        tItem = new QTableWidgetItem(QString::number(tempPacket.fromPort));
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "From Port"), tItem);

        tItem = new QTableWidgetItem(tempPacket.toIP);
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "To IP"), tItem);
        /*
            packetTableHeaders <<"Time" << "From IP" <<"From Port" << "To IP" <<
        "To Port" <<
        "Method"<<"Error" << "ASCII" << "Hex";
        */

        tItem = new QTableWidgetItem(QString::number(tempPacket.port));
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "To Port"), tItem);

        tItem = new QTableWidgetItem(tempPacket.tcpOrUdp);
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "Method"), tItem);

        tItem = new QTableWidgetItem(tempPacket.errorString);
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "Error"), tItem);

        tItem = new QTableWidgetItem(tempPacket.hexToASCII(tempPacket.hexString));
        QSize tSize = tItem->sizeHint();
        tSize.setWidth(200);
        tItem->setSizeHint(tSize);
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "ASCII"), tItem);

        tItem = new QTableWidgetItem(tempPacket.hexString);
        Packet::populateTableWidgetItem(tItem, tempPacket);
        ui->trafficLogTable->setItem(rowCounter, findColumnIndex(lw, "Hex"), tItem);

        rowCounter++;



    }
    ui->trafficLogTable->setSortingEnabled(true);

    ui->trafficLogTable->resizeColumnsToContents();
    ui->trafficLogTable->resizeRowsToContents();
    ui->trafficLogTable->horizontalHeader()->setStretchLastSection( true );

}



void MainWindow::loadPacketsTable()
{

    tableActive = false;
    Packet tempPacket;


    QList<Packet> packetsSavedFiltered;
    packetsSavedFiltered.clear();

    QString filterString = ui->searchLineEdit->text().toLower().trimmed();


    foreach(tempPacket, packetsSaved)
    {

        if(tempPacket.name.toLower().contains(filterString) ||
                tempPacket.hexToASCII(tempPacket.hexString).toLower().contains(filterString) ||
                tempPacket.hexString.toLower().contains(filterString)
                )
        {
            packetsSavedFiltered.append(tempPacket);
            continue;
        }
    }



    ui->packetsTable->clear();


    QListWidget * lw = ui->displayOrderList;
    QListWidgetItem * item;
    QStringList lwStringList;
    lwStringList.clear();
    QString text;
    int size = lw->count();

    QDEBUGVAR(size);
    for(int i = 0; i < size; i++)
    {
        item = lw->item(i);
        text = item->text();
        lwStringList.append(text);
    }

    ui->packetsTable->setColumnCount(lwStringList.size());

    ui->packetsTable->verticalHeader()->show();
    ui->packetsTable->horizontalHeader()->show();
    ui->packetsTable->setHorizontalHeaderLabels(lwStringList);
    if(packetsSavedFiltered.isEmpty())
    {
        ui->packetsTable->setRowCount(0);
    } else {
        ui->packetsTable->setRowCount(packetsSavedFiltered.count());
    }


    unsigned int rowCounter = 0;
    ui->responsePacketBox->clear();
    ui->responsePacketBox->addItem("<Load..>");
    foreach(tempPacket, packetsSavedFiltered)
    {
        populateTableRow(rowCounter, tempPacket);
        rowCounter++;

        ui->responsePacketBox->addItem(tempPacket.name);

    }

    ui->packetsTable->resizeColumnsToContents();
    ui->packetsTable->resizeRowsToContents();
    ui->packetsTable->horizontalHeader()->setStretchLastSection( true );

    tableActive = true;


}

void MainWindow::httpFinished(QNetworkReply* pReply)
{

    QByteArray data=pReply->readAll();
    QString str = QString(data);
    str.truncate(100);
    QDEBUG() <<"finished http. size="<<data.size() << str;

}


MainWindow::~MainWindow()
{
    delete ui;
}

quint64 MainWindow::timeSinceLaunch()
{
    static QTime launchTimer = QTime::currentTime();
    if(launchTimer.isValid())
    {
        return launchTimer.elapsed();

    } else {
        launchTimer.start();
        return 0;
    }

}

void MainWindow::on_packetHexEdit_lostFocus()
{

    qDebug() << __FILE__ << "/" << __LINE__ <<"on_serialHexEdit_lostFocus";

    QString quicktestHex =  ui->packetHexEdit->text();

    ui->packetASCIIEdit->setText(Packet::hexToASCII(quicktestHex));

}

void MainWindow::on_packetASCIIEdit_lostFocus()
{

    QString quicktestASCII =  ui->packetASCIIEdit->text();
    ui->packetHexEdit->setText( Packet::ASCIITohex(quicktestASCII));


    QString quicktestASCII2 =  ui->packetHexEdit->text();

    ui->packetASCIIEdit->setText(Packet::hexToASCII(quicktestASCII2));



    qDebug() << __FILE__ << "/" << __LINE__ <<"on_serialASCIIEdit_lostFocus";

}

void MainWindow::statusBarMessage(const QString &msg, int timeout = 3000, bool override = false)
{
    QString currentMsg = statusBar()->currentMessage();

    if(currentMsg.size() > 10)
    {
        override = true;
    }
    if(currentMsg.size() > 0 && !override)
    {
        statusBar()->showMessage(currentMsg + " / " + msg, timeout * 2);
    } else {
        if (timeout == 0)
            timeout = 3000;
        statusBar()->showMessage(msg, timeout);
    }
}
void MainWindow::sendClick(QString packetName)
{
    QDEBUG() << "send click: " << packetName;
    Packet toSend;


    foreach(toSend, packetsSaved)
    {
        if(toSend.name == packetName)
        {

            if(toSend.toIP.trimmed() == "255.255.255.255") {

                QSettings settings(SETTINGSFILE, QSettings::IniFormat);
                bool sendResponse = settings.value("sendReponse", false).toBool();
                if(sendResponse) {
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Broadcast with responses!");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::No);
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText("You are sending a broadcast packet with responses enabled.\n\nThis could cause traffic flooding. Continue?");
                    int yesno = msgBox.exec();
                    if(yesno == QMessageBox::No) {
                        return;
                    }

                }
            }

            if(toSend.repeat > 0)
            {
                toSend.timestamp = QDateTime::currentDateTime();

                stopResendingButton->setStyleSheet("QPushButton { color: green; } QPushButton::hover { color: #BC810C; } ");
                packetsRepeat.append(toSend);
                stopResendingButton->setText("Resending (" + QString::number(packetsRepeat.size()) + ")");
                stopResending = 0;
            }

            lastSendPacket = toSend;
            statusBarMessage("Send: " + packetName);
            emit sendPacket(toSend);
        }
    }

}

void MainWindow::on_savePacketButton_clicked()
{

    Packet testPacket;
    testPacket.init();
    testPacket.name = ui->packetNameEdit->text().trimmed();
    if(testPacket.name.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setText("Name cannot be blank.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Name is empty.");
        msgBox.exec();
        ui->packetNameEdit->setFocus();
        return;

    }
    testPacket.toIP = ui->packetIPEdit->text().trimmed();
    testPacket.hexString =  ui->packetHexEdit->text().simplified();
    testPacket.tcpOrUdp = ui->udptcpComboBox->currentText();
    testPacket.sendResponse =  0;
    testPacket.port = ui->packetPortEdit->text().toUInt();
    testPacket.repeat = ui->resendEdit->text().toUInt();

    testPacket.saveToDB();
    packetsSaved = Packet::fetchAllfromDB("");

    //ui->searchLineEdit->setText("");
    loadPacketsTable();

}

void MainWindow::on_testPacketButton_clicked()
{
    Packet testPacket;
    testPacket.init();

    if(ui->testPacketButton->text().contains("Multi")) {
        QDEBUG() << "We are multi";
        int packetCount = 0;

        QList<QTableWidgetItem *> totalSelected = ui->packetsTable->selectedItems();
        if(!totalSelected.isEmpty()) {
            QTableWidgetItem * item;
            QList<int> usedRows;
            usedRows.clear();
            foreach(item, totalSelected) {
                if(usedRows.contains(item->row())) {
                    continue;
                } else {
                    usedRows.append(item->row());
                    Packet clickedPacket = Packet::fetchTableWidgetItemData(item);
                    emit sendPacket(clickedPacket);
                    packetCount++;

                }

            }

        }
        statusBarMessage("Sending "+QString::number(packetCount)+" packets");
        return;

    }


    testPacket.name = ui->packetNameEdit->text().trimmed();
    testPacket.toIP = ui->packetIPEdit->text().trimmed();
    testPacket.hexString =  ui->packetHexEdit->text().simplified();
    testPacket.tcpOrUdp = ui->udptcpComboBox->currentText();
    testPacket.sendResponse =  0;
    testPacket.port = ui->packetPortEdit->text().toUInt();
    testPacket.repeat = ui->resendEdit->text().toUInt();


    if(testPacket.toIP.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setText("Address cannot be blank.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Address is empty.");
        msgBox.exec();
        ui->packetIPEdit->setFocus();
        return;

    }


    if(testPacket.port == 0) {

        QMessageBox msgBox;
        msgBox.setText("Port cannot be blank/zero.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Port is zero.");
        msgBox.exec();
        ui->packetPortEdit->setFocus();
        return;

    }

    if(testPacket.toIP.trimmed() == "255.255.255.255") {

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        bool sendResponse = settings.value("sendReponse", false).toBool();
        if(sendResponse) {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Broadcast with responses!");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("You are sending a broadcast packet with responses enabled.\n\nThis could cause traffic flooding. Continue?");
            int yesno = msgBox.exec();
            if(yesno == QMessageBox::No) {
                return;
            }
        }
    }


    lastSendPacket = testPacket;
    lastSendPacket.name.clear();

    if(testPacket.repeat > 0)
    {
        testPacket.timestamp = QDateTime::currentDateTime();

        stopResendingButton->setStyleSheet("QPushButton { color: green; } QPushButton::hover { color: #BC810C; } ");
        packetsRepeat.append(testPacket);
        stopResendingButton->setText("Resending (" + QString::number(packetsRepeat.size()) + ")");
        stopResending = 0;
    }

    emit sendPacket(testPacket);


}

void MainWindow::on_deletePacketButton_clicked()
{
    QModelIndexList indexes = ui->packetsTable->selectionModel()->selectedIndexes();
    QDEBUG() << "Delete packets";
    QModelIndex index;
    QString selected;

    if(indexes.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setText("No packets selected.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("Select a packet.");
        msgBox.exec();
        return;
    }

    foreach(index, indexes)
    {
        selected = index.data(Packet::PACKET_NAME).toString();
        QDEBUG() << "Deleting"<<selected;
        Packet::removeFromDB(selected);
    }

    packetsSaved = Packet::fetchAllfromDB("");
    ui->searchLineEdit->setText("");
    loadPacketsTable();

}



void MainWindow::on_packetIPEdit_lostFocus()
{
    QString ipPacket = ui->packetIPEdit->text().trimmed();
    QHostAddress address(ipPacket);
    if (QAbstractSocket::IPv4Protocol == address.protocol())
    {
       qDebug("Valid IPv4 address.");
    }
    else if (QAbstractSocket::IPv6Protocol == address.protocol())
    {
       qDebug("Valid IPv6 address.");
    }
    else
    {
        QHostInfo info = QHostInfo::fromName(ipPacket);
        if (info.error() != QHostInfo::NoError)
        {
            ui->packetIPEdit->setText("");
            ui->packetIPEdit->setPlaceholderText("Invalid Address / DNS failed");
        } else {
            ui->packetIPEdit->setText(info.addresses().at(0).toString());
        }
    }
}

void MainWindow::on_packetPortEdit_lostFocus()
{
    QString portPacket = ui->packetPortEdit->text().trimmed();
    unsigned int port =0 ;
    bool ok;
    port = portPacket.toUInt(&ok, 0);
    QDEBUGVAR(port);
    if(port <= 0 )
    {
        ui->packetPortEdit->setText("");
        ui->packetPortEdit->setPlaceholderText("Invalid Port");
    } else {
        ui->packetPortEdit->setText(QString::number(port));
    }
}

 //   QDEBUG() << "cell changed";

void MainWindow::removePacketfromMemory(Packet thepacket)
{
    for(int i=0; i<packetsSaved.size();i++)
    {
        if(thepacket.name == packetsSaved[i].name)
        {
            packetsSaved.removeAt(i);
            break;
        }

    }

}

void MainWindow::on_packetsTable_itemChanged(QTableWidgetItem *item)
{
    if(!tableActive)
    {
        return;
    }
    tableActive =  false;
    QString datatype = item->data(Packet::DATATYPE).toString();
    QString newText = item->text();
    QDEBUG() << "cell changed:" <<datatype << item->text();
    int fullrefresh = 0;

    Packet updatePacket = Packet::fetchTableWidgetItemData(item);
    if(datatype == "name")
    {
        Packet::removeFromDB(updatePacket.name); //remove old before inserting new.
        removePacketfromMemory(updatePacket);
        updatePacket.name = newText;
        fullrefresh = 1;
    }
    if(datatype == "toIP")
    {
        QHostAddress address(newText);
        if (QAbstractSocket::IPv4Protocol == address.protocol())
        {
            updatePacket.toIP = newText;
        }        else if (QAbstractSocket::IPv6Protocol == address.protocol())
        {
            updatePacket.toIP = newText;
        } else {
            QHostInfo info = QHostInfo::fromName(newText);
            if (info.error() == QHostInfo::NoError)
            {

                updatePacket.toIP = (info.addresses().at(0).toString());
            }

        }


    }
    if(datatype == "port")
    {
        int portNum = newText.toUInt();
        if(portNum > 0)
        {
            updatePacket.port = portNum;
        }
    }
    if(datatype == "repeat")
    {
        int repeat = newText.toUInt();
        updatePacket.repeat = repeat;
    }
    if(datatype == "tcpOrUdp")
    {
        if(newText.trimmed().toUpper() == "TCP" || newText.trimmed().toUpper() == "UDP" )
        {
            updatePacket.tcpOrUdp = newText.trimmed().toUpper();
        }
    }
    if(datatype == "ascii")
    {
        QString hex = Packet::ASCIITohex(newText);
        updatePacket.hexString = hex;
    }
    if(datatype == "hexString")
    {
        QString hex = newText;
        QString ascii = Packet::hexToASCII(newText);
        updatePacket.hexString = newText;
    }

    updatePacket.saveToDB();
    packetsSaved = Packet::fetchAllfromDB("");

    if(fullrefresh)
    {
        loadPacketsTable();
    } else {
        populateTableRow(item->row(), updatePacket);

    }
    tableActive =  true;
}



void MainWindow::poodlepic()
{
    QDEBUG();

    BruceThePoodle *bruce = new BruceThePoodle(this);
    bruce->show();
}


//packetSavedTableHeaders <<"Send" <<"Resend (s)"<< "Name" << "To Address" << "To Port" << "Method" << "ASCII" << "Hex";


int MainWindow::findColumnIndex(QListWidget * lw, QString search)
{
    QListWidgetItem * item;
    QString text;
    int size = lw->count();

    for(int i = 0; i < size; i++)
    {
        item = lw->item(i);
        text = item->text();
        if(text == search)
        {
            return i;
        }

    }
    QDEBUGVAR(search);
    return -1;

}

void MainWindow::populateTableRow(int rowCounter, Packet tempPacket)
{
    QTableWidgetItem * tItem;
    QListWidget *lw;

    SendPacketButton * sendButton = tempPacket.getSendButton(ui->packetsTable);


    connect(sendButton, SIGNAL(sendPacket(QString)),
        this, SLOT(sendClick(QString)));

    //http->get(QNetworkRequest(QUrl("http://packetsender.com/")));

    /*
     *
     *
    packetSavedTableHeaders <<"Send" <<"Resend (sec)"<< "Name" << "To Address" <<
                "To Port" << "Method" << "ASCII" << "Hex";
    packetTableHeaders.clear();
    packetTableHeaders <<"Time" << "From IP" <<"From Port" << "To IP" << "To Port" << "Method"
                <<"Error" << "ASCII" << "Hex";

     */

    lw = ui->displayOrderList;

    ui->packetsTable->setCellWidget(rowCounter,findColumnIndex(lw, "Send"), sendButton);

    tItem = new QTableWidgetItem(QString::number(tempPacket.repeat));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "repeat");
    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "Resend (sec)"), tItem);
    //QDEBUGVAR(tempPacket.name);

    tItem = new QTableWidgetItem(tempPacket.name);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "name");
    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "Name"), tItem);
    //QDEBUGVAR(tempPacket.name);

    tItem = new QTableWidgetItem(tempPacket.toIP);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "toIP");

    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "To Address"), tItem);
    //QDEBUGVAR(tempPacket.toIP);

    tItem = new QTableWidgetItem(QString::number(tempPacket.port));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "port");
    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "To Port"), tItem);
   // QDEBUGVAR(tempPacket.port);

    tItem = new QTableWidgetItem(tempPacket.tcpOrUdp);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "tcpOrUdp");
    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "Method"), tItem);
   // QDEBUGVAR(tempPacket.tcpOrUdp);

    tItem = new QTableWidgetItem(tempPacket.hexToASCII(tempPacket.hexString));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "ascii");

    QSize tSize = tItem->sizeHint();
    tSize.setWidth(200);
    tItem->setSizeHint(tSize);

    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "ASCII"), tItem);
    //QDEBUGVAR(tempPacket.hexString);

    tItem = new QTableWidgetItem(tempPacket.hexString);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "hexString");
    ui->packetsTable->setItem(rowCounter, findColumnIndex(lw, "Hex"), tItem);
    //QDEBUGVAR(tempPacket.hexString);
}

void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                delete widget;
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void MainWindow::packetTable_checkMultiSelected() {

    //how many are selected?
    QTableWidgetItem * checkItem;
    SendPacketButton *sendButton;
    QList<Packet> packetList;
    Packet clickedPacket;
    QList<QTableWidgetItem *> totalSelected = ui->packetsTable->selectedItems();
    packetList.clear();
    QStringList buttonsList;
    buttonsList.clear();
    int i = 0;


    //clear out old, if any.
    foreach(sendButton, sendButtonHolder) {
        QDEBUG() << "Removing" << sendButton->text();
        ui->selectedbuttonlayout->removeWidget(sendButton);
        sendButton->deleteLater();
    }
    sendButtonHolder.clear();

    foreach(checkItem, totalSelected) {
        clickedPacket = Packet::fetchTableWidgetItemData(checkItem);
        if(buttonsList.contains(clickedPacket.name)) {
            continue;
        } else {
            packetList.append(clickedPacket);
            buttonsList.append(clickedPacket.name);
        }
    }

    int vert = 0;
    if(!packetList.isEmpty()) {
        ui->selectedButtonsLabel->hide();
        QDEBUG() << "Need to add" << packetList.size();

        foreach(clickedPacket, packetList) {
            sendButton = clickedPacket.getSendButton(ui->packetsTable);
            sendButton->setText(clickedPacket.name);
            vert = sendButton->size().height();

            connect(sendButton, SIGNAL(sendPacket(QString)),
                this, SLOT(sendClick(QString)));

                    //new QPushButton(clickedPacket.name);
            QDEBUG() << "Adding" << (clickedPacket.name);
            i++;
            QDEBUGVAR( ui->selectedbuttonlayout->count());
            ui->selectedbuttonlayout->addWidget(sendButton);
            QDEBUGVAR( ui->selectedbuttonlayout->count());
            sendButtonHolder.append(sendButton);
        }

    } else {
        if(!ui->hideQuickSendCheck->isChecked()) {
            ui->selectedButtonsLabel->show();
        }
    }

//    ui->buttonScrollArea->resize(ui->buttonScrollArea->size().width(), vert+5);

    QDEBUG() <<"Added" << i << "size is" << ui->selectedbuttonlayout->count();

    if(packetList.size() >= 2) {
        //We have multi!
        ui->testPacketButton->setText("Multi-Send");
        ui->testPacketButton->setStyleSheet("color:green;");
        ui->packetASCIIEdit->setEnabled(false);
        ui->packetNameEdit->setEnabled(false);
        ui->packetHexEdit->setEnabled(false);
        ui->packetIPEdit->setEnabled(false);
        ui->packetPortEdit->setEnabled(false);
        ui->udptcpComboBox->setEnabled(false);
        ui->savePacketButton->setEnabled(false);
        ui->resendEdit->setEnabled(false);
        return;

    }


    ui->testPacketButton->setText("Send");
    ui->testPacketButton->setStyleSheet("");

    ui->packetNameEdit->setEnabled(true);
    ui->packetASCIIEdit->setEnabled(true);
    ui->packetHexEdit->setEnabled(true);
    ui->packetIPEdit->setEnabled(true);
    ui->packetPortEdit->setEnabled(true);
    ui->udptcpComboBox->setEnabled(true);
    ui->savePacketButton->setEnabled(true);
    ui->resendEdit->setEnabled(true);


}

void MainWindow::on_packetsTable_itemClicked(QTableWidgetItem *item)
{


    packetTable_checkMultiSelected();


    if(item->isSelected())
    {
        Packet clickedPacket = Packet::fetchTableWidgetItemData(item);

        if(item->column() != 0)
        {
            ui->packetNameEdit->setText(clickedPacket.name);
            ui->packetHexEdit->setText(clickedPacket.hexString);
            ui->packetIPEdit->setText(clickedPacket.toIP);
            ui->packetPortEdit->setText(QString::number(clickedPacket.port));
            ui->resendEdit->setText(QString::number(clickedPacket.repeat));
            ui->udptcpComboBox->setCurrentIndex(ui->udptcpComboBox->findText(clickedPacket.tcpOrUdp));
            ui->packetASCIIEdit->setText(Packet::hexToASCII(clickedPacket.hexString));
        }
    }
}

void MainWindow::refreshTimerTimeout()
{
    //QDEBUGVAR(sendButtonHolder.size());
    //QDEBUGVAR(ui->selectedbuttonlayout->count());
    //QDEBUGVAR(packetsLogged.size());
    static QStringList packetSavedHeaderBefore;
    static QStringList packetSavedHeaderBeforeTraffic;
    QListWidget * lw = ui->displayOrderList;
    QListWidget * lwTraffic = ui->displayOrderListTraffic;

    //Too bad Qt does not have a drag/drop signal.
    //That would be more efficient than constantly polling for a change.
    //I think one could be implemented, but this hack was quicker.

    QStringList packetSavedHeaderNow;
    packetSavedHeaderNow.clear();
    for(int i = 0; i < lw->count(); i++)
    {
        packetSavedHeaderNow.append(lw->item(i)->text());
    }

    if(packetSavedHeaderBefore != packetSavedHeaderNow)
    {
        QSettings settings1(SETTINGSFILE, QSettings::IniFormat);
        settings1.setValue("packetSavedTableHeaders", packetSavedHeaderNow);

        loadPacketsTable();
        packetSavedHeaderBefore = packetSavedHeaderNow;
    }

    packetSavedHeaderNow.clear();
    for(int i = 0; i < lwTraffic->count(); i++)
    {
        packetSavedHeaderNow.append(lwTraffic->item(i)->text());
    }

    if(packetSavedHeaderBeforeTraffic != packetSavedHeaderNow)
    {
        QSettings settings2(SETTINGSFILE, QSettings::IniFormat);
        settings2.setValue("packetTableHeaders", packetSavedHeaderNow);

        loadTrafficLogTable();
        packetSavedHeaderBeforeTraffic = packetSavedHeaderNow;
    }



    Packet packet;
    QDateTime now = QDateTime::currentDateTime();

    for(int i = 0; i < packetsRepeat.size() && !stopResending; i++)
    {
            if(packetsRepeat[i].timestamp.addMSecs(packetsRepeat[i].repeat * 1000 - 100) < now)
            {
                packetsRepeat[i].timestamp = now;
                emit sendPacket(packetsRepeat[i]);


                statusBarMessage("Send: " + packetsRepeat[i].name + " (Resend)");
            }
    }


    if(packetsRepeat.isEmpty() || stopResending)
    {
        stopResendingButton->hide();
        packetsRepeat.clear();
        stopResending = 0;

    } else {
        stopResendingButton->show();
    }


    if(ui->trafficLogTable->rowCount() != packetsLogged.size())
    {
        while(maxLogSize > 0 && packetsLogged.size() > maxLogSize) {
            packetsLogged.removeFirst();
        }

        loadTrafficLogTable();

        ui->mainTabWidget->setTabText(1,"Traffic Log (" + QString::number(packetsLogged.size()) +")");

    }


}


void MainWindow::gotoDanNagleDotCom()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://dannagle.com/"));

}
void MainWindow::gotoDanNaglePayPal()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://dannagle.com/paypal"));

}

void MainWindow::gotoNagleCode()
{
    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://twitter.com/naglecode"));


}

void MainWindow::gotoPacketSenderDotCom()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://packetsender.com/"));

}

void MainWindow::on_trafficLogClearButton_clicked()
{
    ui->trafficLogTable->clear();
    ui->trafficLogTable->setRowCount(0);
    packetsLogged.clear();
    loadTrafficLogTable();
    ui->mainTabWidget->setTabText(1,"Traffic Log");

}


void MainWindow::on_saveTrafficPacket_clicked()
{
    QList<QTableWidgetItem *> items = ui->trafficLogTable->selectedItems();
    QDEBUG() << "Save traffic packets";
    QTableWidgetItem *item;
    QString selected;


    Packet savePacket;
    QString namePrompt;
    bool ok;
    foreach(item, items)
    {
        savePacket = Packet::fetchTableWidgetItemData(item);
        QDEBUG() << "Saving"<<savePacket.name;
        namePrompt = QInputDialog::getText(this, tr("Save Packet"),
                                                  tr("Packet name:"), QLineEdit::Normal,
                                                  savePacket.name, &ok);
        if (ok && !namePrompt.isEmpty())
        {
            savePacket.name = namePrompt.trimmed();
        }


        if(savePacket.toIP.toUpper().trimmed() == "YOU")
        {
            savePacket.toIP = savePacket.fromIP;
            savePacket.port = savePacket.fromPort;
        }

        savePacket.repeat = 0;

        savePacket.saveToDB();
        break;
    }

    packetsSaved = Packet::fetchAllfromDB("");
    ui->searchLineEdit->setText("");
    loadPacketsTable();


}
void MainWindow::applyNetworkSettings()
{
    if(ui->mainTabWidget->currentIndex() != 2) {
        return;
    }

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    settings.setValue("udpPort", ui->udpServerPortEdit->text().toUInt());
    settings.setValue("tcpPort", ui->tcpServerPortEdit->text().toUInt());
    settings.setValue("sendReponse", ui->sendResponseSettingsCheck->isChecked());
    settings.setValue("responseName", ui->responsePacketBox->currentText().trimmed());
    settings.setValue("responseHex", ui->hexResponseEdit->text().trimmed());

    settings.setValue("udpServerEnable", ui->udpServerEnableCheck->isChecked());
    settings.setValue("tcpServerEnable", ui->tcpServerEnableCheck->isChecked());
    settings.setValue("attemptReceiveCheck", ui->attemptReceiveCheck->isChecked());

    settings.setValue("delayAfterConnectCheck", ui->delayAfterConnectCheck->isChecked());
    settings.setValue("persistentConnectCheck", ui->persistentConnectCheck->isChecked());


    packetNetwork.kill();
    packetNetwork.init();
    packetNetwork.responseData = ui->hexResponseEdit->text().trimmed();
    packetNetwork.sendResponse = ui->sendResponseSettingsCheck->isChecked();
    UDPServerStatus();
    TCPServerStatus();

}

void MainWindow::cancelResends()
{
    stopResendingButton->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    stopResendingButton->setText("Resending");
    stopResending = 1;
}


void MainWindow::toggleUDPServer()
{
    ui->udpServerEnableCheck->setChecked(!ui->udpServerEnableCheck->isChecked());
    on_udpServerEnableCheck_toggled(ui->udpServerEnableCheck->isChecked());
//    applyNetworkSettings();
}
void MainWindow::toggleTCPServer()
{
    ui->tcpServerEnableCheck->setChecked(!ui->tcpServerEnableCheck->isChecked());
    on_tcpServerEnableCheck_toggled(ui->tcpServerEnableCheck->isChecked());
//    applyNetworkSettings();
}

void MainWindow::on_tcpServerPortEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    applyNetworkSettings();

}

void MainWindow::on_udpServerPortEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    applyNetworkSettings();

}

void MainWindow::on_sendResponseSettingsCheck_toggled(bool checked)
{
    Q_UNUSED(checked);
    applyNetworkSettings();
}

void MainWindow::on_responsePacketBox_activated(int index)
{
    Q_UNUSED(index);
    applyNetworkSettings();
}


void MainWindow::on_udpServerEnableCheck_toggled(bool checked)
{
    Q_UNUSED(checked);
    applyNetworkSettings();
}


void MainWindow::on_tcpServerEnableCheck_toggled(bool checked)
{
    Q_UNUSED(checked);
    applyNetworkSettings();
}


void MainWindow::on_packetASCIIEdit_editingFinished()
{
    on_packetASCIIEdit_lostFocus();
}

void MainWindow::on_packetHexEdit_editingFinished()
{
    on_packetHexEdit_lostFocus();
}

void MainWindow::on_packetASCIIEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

}

void MainWindow::on_packetIPEdit_editingFinished()
{
    on_packetIPEdit_lostFocus();
}

void MainWindow::on_hexResponseEdit_editingFinished()
{


    QString quicktestHex =  ui->hexResponseEdit->text();

    ui->asciiResponseEdit->setText(Packet::hexToASCII(quicktestHex));
    applyNetworkSettings();

}

void MainWindow::on_asciiResponseEdit_editingFinished()
{

    QString quicktestASCII =  ui->asciiResponseEdit->text();
    ui->hexResponseEdit->setText( Packet::ASCIITohex(quicktestASCII));

     QString quicktestASCII2 =  ui->hexResponseEdit->text();

    ui->asciiResponseEdit->setText(Packet::hexToASCII(quicktestASCII2));
    applyNetworkSettings();



}

void MainWindow::on_responsePacketBox_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    Packet tempPacket;
    QString selectedName = ui->responsePacketBox->currentText();

    //QDEBUGVAR(selectedName);
    foreach(tempPacket, packetsSaved)
    {
        if(tempPacket.name == selectedName)
        {
            QByteArray hex = tempPacket.response;
            ui->hexResponseEdit->setText(tempPacket.hexString);
            on_hexResponseEdit_editingFinished();
            break;
        }

    }

    ui->responsePacketBox->setCurrentIndex(0);
    applyNetworkSettings();

}

void MainWindow::on_searchLineEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

    loadPacketsTable();

}

void MainWindow::on_clearSearch_clicked()
{
    ui->searchLineEdit->clear();
    on_searchLineEdit_textEdited("");
}

void MainWindow::dragDropEvent()
{
    QDEBUG();
}

void MainWindow::on_displayOrderList_indexesMoved(const QModelIndexList &indexes)
{
    Q_UNUSED(indexes);
}

void MainWindow::on_displayOrderList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
}

void MainWindow::on_searchTrafficEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    loadTrafficLogTable();

}

void MainWindow::on_clearTrafficSearchButton_clicked()
{
    ui->searchTrafficEdit->clear();
    loadTrafficLogTable();
}

void MainWindow::on_toClipboardButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    QList<QTableWidgetItem *> items = ui->trafficLogTable->selectedItems();
    QDEBUG() << "Save traffic packets";
    QTableWidgetItem *item;
    QString selected;


    Packet savePacket;
    QString clipString;
    clipString.clear();
    QTextStream out;
    out.setString(&clipString);

    if(items.size() == 0)
    {

        QMessageBox msgBox;
        msgBox.setText("No packets selected.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("Clipboard unchanged.");
        msgBox.exec();
        return;
    }
    foreach(item, items)
    {
        savePacket = Packet::fetchTableWidgetItemData(item);
        out << "Time: " << savePacket.timestamp.toString(DATETIMEFORMAT) << "\n";
        out << "TO: " << savePacket.toIP << ":" << savePacket.port<< "\n";
        out << "From: " << savePacket.fromIP << ":" << savePacket.fromPort << "\n";
        out << "Method: " << savePacket.tcpOrUdp << "\n";
        out << "Error: " << savePacket.errorString;
        out <<"\n\nASCII:"<<"\n";
        out << savePacket.hexToASCII(savePacket.hexString) << "\n";
        out <<"\n\nHEX:"<<"\n";
        out << savePacket.hexString << "\n";

        break;

    }

    QMessageBox msgBox;
    if(ui->copyUnformattedCheck->isChecked()) {
        clipboard->setText(QString(savePacket.getByteArray()));
        msgBox.setText("Packet sent to your clipboard (raw data).");
    } else {
        clipboard->setText(clipString);
        msgBox.setText("Packet sent to your clipboard (translated).");
    }


    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("To Clipboard.");
    msgBox.exec();


}


void MainWindow::setDefaultTableHeaders() {

    packetSavedTableHeaders.clear();
    packetSavedTableHeaders <<"Send" << "Name"<<"Resend (sec)" << "To Address" << "To Port" << "Method" << "ASCII" << "Hex";
    packetTableHeaders.clear();
    packetTableHeaders <<"Time" << "From IP" <<"From Port" << "To IP" << "To Port" << "Method"<<"Error" << "ASCII" << "Hex";

}

void MainWindow::setStoredTableHeaders() {

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QStringList packetSavedTable = settings.value("packetSavedTableHeaders", packetSavedTableHeaders).toStringList();
    QStringList packetTable = settings.value("packetTableHeaders", packetTableHeaders).toStringList();


    if(packetSavedTable.size() == packetSavedTableHeaders.size()) {
        packetSavedTableHeaders = packetSavedTable;
    }

    if(packetTable.size() == packetTableHeaders.size()) {
        packetTableHeaders = packetTable;
    }

}

void MainWindow::loadTableHeaders()
{

    QString tempString;
    tempString.clear();

    QListWidgetItem * tItem;

    ui->displayOrderList->clear();
    ui->displayOrderListTraffic->clear();

    foreach(tempString, packetSavedTableHeaders)
    {
        tItem = new QListWidgetItem(tempString);
        tItem->setIcon(QIcon(UPDOWNICON));
        ui->displayOrderList->addItem(tItem);
    }

    ui->displayOrderList->setCursor(Qt::CrossCursor);

    foreach(tempString, packetTableHeaders)
    {
        tItem = new QListWidgetItem(tempString);
        tItem->setIcon(QIcon(UPDOWNICON));
        ui->displayOrderListTraffic->addItem(tItem);
    }

    ui->displayOrderListTraffic->setCursor(Qt::CrossCursor);

    ui->displayOrderList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->displayOrderList->setAlternatingRowColors(true);
    ui->displayOrderListTraffic->setDragDropMode(QAbstractItemView::InternalMove);
    ui->displayOrderListTraffic->setAlternatingRowColors(true);

}


void MainWindow::on_defaultDisplayButton_clicked()
{
    setDefaultTableHeaders();
    loadTableHeaders();
}

void MainWindow::on_packetsTable_itemSelectionChanged()
{
    packetTable_checkMultiSelected();

}


void MainWindow::on_attemptReceiveCheck_clicked(bool checked)
{
    Q_UNUSED(checked);
    applyNetworkSettings();
}

void MainWindow::on_rolling500entryCheck_clicked(bool checked)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    if(checked) {
        maxLogSize = 100;
    } else {
        maxLogSize = 0;
    }

    settings.setValue("rolling500entryCheck", checked);

}

void MainWindow::on_copyUnformattedCheck_clicked(bool checked)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("copyUnformattedCheck", checked);
}

void MainWindow::on_delayAfterConnectCheck_clicked(bool checked)
{
    Q_UNUSED(checked);
    applyNetworkSettings();

}

void MainWindow::on_persistentConnectCheck_clicked(bool checked)
{

    if(checked) {

        QMessageBox msgBox;
        msgBox.setWindowTitle("Persistent Connection");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("This spawns a new window for each TCP connection. \n\nClose the window to close the connection.");
        msgBox.exec();
    }

    applyNetworkSettings();

}

void MainWindow::on_bugsLinkButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://bugtracker.dannagle.com/"));
}


void MainWindow::on_forumsPacketSenderButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://forums.packetsender.com/"));


}

void MainWindow::on_exportPacketsButton_clicked()
{
    static QString fileName = QDir::homePath() + QString("/packetsender_export.ini");

    fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               QDir::toNativeSeparators(fileName), tr("INI db (*.ini)"));

   QStringList testExt = fileName.split(".");
   QString ext = "";
   if(testExt.size() > 0) {
       ext = testExt.last();
   }
   if(ext != "ini") {
       fileName.append(".ini");
   }
   QDEBUGVAR(fileName);
   QFile::copy(PACKETSFILE, fileName);
   statusBarMessage("Export: " + fileName);

}

void MainWindow::on_importPacketsButton_clicked()
{
    static QString fileName = QDir::homePath() + QString("/packetsender_export.ini");


    fileName = QFileDialog::getOpenFileName(this, tr("Import File"),
                                              fileName,
                                              tr("INI db (*.ini)"));

    QDEBUGVAR(fileName);
    QList<Packet> importList = Packet::fetchAllfromDB(fileName);
    Packet importPacket;

    foreach(importPacket, importList) {
        QDEBUGVAR(importPacket.name);
    }

    if(!importList.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Found " + QString::number(importList.size()) + " packets!");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Import " + QString::number(importList.size()) + " packets?\n\nPacket Sender will overwrite packets with the same name.");
        int yesno = msgBox.exec();
        if(yesno == QMessageBox::No) {
            statusBarMessage("Import Cancelled");
            return;
        } else {
            foreach(importPacket, importList) {
                importPacket.saveToDB();

            }
            packetsSaved = Packet::fetchAllfromDB("");
            loadPacketsTable();
            statusBarMessage("Import Finished");
        }

    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Not a database");
        msgBox.setStandardButtons(QMessageBox::Ok );
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Found no packets in this file. It may not be a Packet Sender export");
        msgBox.exec();
        return;
        statusBarMessage("Import Cancelled");
    }

}

void MainWindow::on_hideQuickSendCheck_clicked(bool checked)
{

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("hideQuickSendCheck", checked);
    if(checked) {
        ui->buttonScrollArea->hide();
    } else {
        ui->buttonScrollArea->show();
    }

}

void MainWindow::on_saveLogButton_clicked()
{
    static QString fileName = QDir::homePath() + QString("/trafficlog.log");

    fileName = QFileDialog::getSaveFileName(this, tr("Save Traffic Log"),
                               QDir::toNativeSeparators(fileName), tr("log (*.log)"));

   QStringList testExt = fileName.split(".");
   QString ext = "";
   if(testExt.size() > 0) {
       ext = testExt.last();
   }
   if(ext != "log") {
       fileName.append(".log");
   }
   QDEBUG() << "Export log to" << fileName;

   QString exportString = "";
   QString delim = "\t";

   QTextStream out;
   out.setString(&exportString);

   out << "TIME" << delim << "From IP" << delim << "From Port" << delim << "To IP"
       << delim << "To Port" << delim << "Method" << delim << "Error" << delim << "ASCII" << delim << "Hex\n";


   Packet tempPacket;
   foreach(tempPacket, packetsLogged)
   {

       exportString.append(tempPacket.timestamp.toString(DATETIMEFORMAT));exportString.append(delim);
       exportString.append(tempPacket.fromIP);exportString.append(delim);
       exportString.append(QString::number(tempPacket.fromPort));exportString.append(delim);
       exportString.append(tempPacket.toIP);exportString.append(delim);
       exportString.append(QString::number(tempPacket.port));exportString.append(delim);
       exportString.append(tempPacket.tcpOrUdp);exportString.append(delim);
       exportString.append(tempPacket.errorString);exportString.append(delim);
       exportString.append(tempPacket.hexToASCII(tempPacket.hexString));exportString.append(delim);
       exportString.append(tempPacket.hexString);exportString.append(delim);
       exportString.append("\n");
   }


   QFile file(fileName);
   if(file.open(QFile::WriteOnly)) {
       file.write(exportString.toLatin1());
       file.close();
   }

   statusBarMessage("Save Log: " + fileName);

}

void MainWindow::on_persistentConnectCheck_toggled(bool checked)
{
}
