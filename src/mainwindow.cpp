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
#include <QSslKey>


#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QStringList>


#include "brucethepoodle.h"
#include "settings.h"
#include "about.h"
#include "subnetcalc.h"
#include "udpflooding.h"
#include "cloudui.h"


int hexToInt(QChar hex);
void parserMajorMinorBuild(QString sw, unsigned int &major, unsigned int &minor, unsigned int &build);
void themeTheButton(QPushButton * button);


//Only AppImage linux needs to check for updates.

#ifdef __linux__

#define SNAPBUILD true

#else

#define SNAPBUILD false

#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timeSinceLaunch();

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QIcon mIcon(":pslogo.png");


    lastSendPacket.clear();

    setWindowTitle("Packet Sender");
    setWindowIcon(mIcon);

    tableActive = false;


    maxLogSize = 10000;

    if (settings.value("rolling500entryCheck", false).toBool()) {
        maxLogSize = 100;
    }


    http = new QNetworkAccessManager(this); //Main application http object

    QDEBUG() << " http connect attempt:" << connect(http, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(httpFinished(QNetworkReply*)));
    //http->get(QNetworkRequest(QUrl("http://packetsender.com/")));


    QDEBUG() << " packet send connect attempt:" << connect(this, SIGNAL(sendPacket(Packet)),
             &packetNetwork, SLOT(packetToSend(Packet)));

    packetNetwork.init();

    packetNetwork.responseData = settings.value("responseHex", "").toString();

    ui->persistentTCPCheck->setChecked(settings.value("persistentTCPCheck", false).toBool());
    packetNetwork.persistentConnectCheck = ui->persistentTCPCheck->isChecked();

    ui->packetsTable->setWordWrap(false);
    ui->trafficLogTable->setWordWrap(false);

    //load last session
    if (settings.value("restoreSessionCheck", true).toBool()) {
        QDEBUG() << "Restoring last session";
        //ui->packetNameEdit->setText(settings.value("packetNameEditSession","").toString());
        ui->packetIPEdit->setText(settings.value("packetIPEditSession", "").toString());
        ui->packetHexEdit->setText(settings.value("packetHexEditSession", "").toString());
        QString methodchoice = settings.value("udptcpComboBoxSession", "TCP").toString();
        int findtext = ui->udptcpComboBox->findText(methodchoice);
        if (findtext > -1) {
            ui->udptcpComboBox->setCurrentIndex(findtext);
        }
        ui->packetPortEdit->setText(settings.value("packetPortEditSession", "").toString());
        ui->resendEdit->setText(settings.value("resendEditSession", "").toString());

        if (!ui->packetHexEdit->text().isEmpty()) {
            on_packetHexEdit_lostFocus();
        }

    }




    packetNetwork.sendResponse = settings.value("sendReponse", false).toBool();


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
    QDEBUGVAR(packetsSaved.size());

    packetTableHeaders  = Settings::defaultTrafficTableHeader();
    packetTableHeaders = settings.value("packetTableHeaders", packetTableHeaders).toStringList();
    packetsLogged.setTableHeaders(packetTableHeaders);

    ui->trafficLogTable->setModel(&packetsLogged);


    ui->trafficLogTable->verticalHeader()->show();
    ui->trafficLogTable->horizontalHeader()->show();

    loadPacketsTable();

//   statusBar()->insertPermanentWidget(0, generatePSLink());
//   statusBar()->insertPermanentWidget(1, generateDNLink());



    stopResendingButton = new QPushButton("Resending");
    stopResendingButton->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    themeTheButton(stopResendingButton);
    stopResendingButton->setIcon(QIcon(PSLOGO));

    connect(stopResendingButton, SIGNAL(clicked()),
            this, SLOT(cancelResends()));


    statusBar()->insertPermanentWidget(1, stopResendingButton);

    stopResendingButton->hide();

    udpServerStatus = new QPushButton("UDP:" + packetNetwork.getUDPPortString());
    themeTheButton(udpServerStatus);
    udpServerStatus->setIcon(QIcon(UDPRXICON));

    connect(udpServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleUDPServer()));


    statusBar()->insertPermanentWidget(2, udpServerStatus);


    //updatewidget
    tcpServerStatus = new QPushButton("TCP:" + (packetNetwork.getTCPPortString()));
    themeTheButton(tcpServerStatus);
    tcpServerStatus->setIcon(QIcon(TCPRXICON));

    sslServerStatus = new QPushButton("SSL:" + (packetNetwork.getSSLPortString()));
    themeTheButton(sslServerStatus);
    sslServerStatus->setIcon(QIcon(SSLRXICON));


    connect(tcpServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleTCPServer()));

    connect(sslServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleSSLServer()));


    statusBar()->insertPermanentWidget(3, tcpServerStatus);


    statusBar()->insertPermanentWidget(4, sslServerStatus);

    IPv4Stylesheet = "QPushButton {width:50px; color: green; } QPushButton::hover { color: #BC810C; } ";
    IPv6Stylesheet = "QPushButton {width:50px; color: blue; } QPushButton::hover { color: #BC810C; } ";

    //ipmode toggle
    IPmodeButton = new QPushButton("IPv4 Mode");
    themeTheButton(IPmodeButton);
    statusBar()->insertPermanentWidget(5, IPmodeButton);

    setIPMode();


    connect(IPmodeButton, SIGNAL(clicked()),
            this, SLOT(toggleIPv4_IPv6()));


    UDPServerStatus();
    TCPServerStatus();
    SSLServerStatus();

    multiSendDelay = settings.value("multiSendDelay", 0).toFloat();
    cancelResendNum = settings.value("cancelResendNum", 0).toInt();
    resendCounter = 0;


    //every 15 seconds
    slowRefreshTimer.setInterval(5000);
    connect(&slowRefreshTimer, SIGNAL(timeout()), this, SLOT(slowRefreshTimerTimeout())) ;
    slowRefreshTimer.start();
    slowRefreshTimerTimeout();


    //sending and logging
    refreshTimer.setInterval(90);
    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout())) ;
    refreshTimer.start();


    packetsLogged.clear();


    generateConnectionMenu();


    if (packetsSaved.size() > 0) {
        ui->searchLineEdit->setFocus(); //put cursor in search bar
    }



    //Bruce is my pet poodle.
    //Dog easter egg.  CTRL D, O, G.
    //             or  CMD D, O, G.
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D, Qt::CTRL + Qt::Key_O, Qt::CTRL + Qt::Key_G), this);
    QDEBUG() << ": dog easter egg Connection attempt " << connect(shortcut, SIGNAL(activated()), this, SLOT(poodlepic()));

    QShortcut *field1 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_1), this);
    QShortcut *field2 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_2), this);
    QShortcut *field3 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_3), this);
    QShortcut *field4 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_4), this);
    QShortcut *field5 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_5), this);
    QShortcut *field6 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_6), this);
    QShortcut *field7 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_7), this);
    QShortcut *field8 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_8), this);


    if (!connect(field1, &QShortcut::activated, this, &MainWindow::shortcutkey1)) {
        QDEBUG() << "field1 connection false";
    }

    if (!connect(field2, &QShortcut::activated, this, &MainWindow::shortcutkey2)) {
        QDEBUG() << "field2 connection false";
    }

    if (!connect(field3, &QShortcut::activated, this, &MainWindow::shortcutkey3)) {
        QDEBUG() << "field3 connection false";
    }

    if (!connect(field4, &QShortcut::activated, this, &MainWindow::shortcutkey4)) {
        QDEBUG() << "field4 connection false";
    }

    if (!connect(field5, &QShortcut::activated, this, &MainWindow::shortcutkey5)) {
        QDEBUG() << "field5 connection false";
    }
    if (!connect(field6, &QShortcut::activated, this, &MainWindow::shortcutkey6)) {
        QDEBUG() << "field6 connection false";
    }

    if (!connect(field7, &QShortcut::activated, this, &MainWindow::shortcutkey7)) {
        QDEBUG() << "field7 connection false";
    }

    if (!connect(field8, &QShortcut::activated, this, &MainWindow::on_testPacketButton_clicked)) {
        QDEBUG() << "field8 connection false";
    }


    //Now that the UI is loaded, create the settings folders if they do not exist
    QDir mdir;
    mdir.mkpath(TEMPPATH);
    mdir.mkpath(SETTINGSPATH);



    //this is stored as base64 so smart git repos
    //do not complain about shipping a private key.
    QFile snakeoilKey("://ps.key.base64");
    QFile snakeoilCert("://ps.pem.base64");


    QString defaultCertFile = CERTFILE;
    QString defaultKeyFile = KEYFILE;

    QFile certfile(defaultCertFile);
    QFile keyfile(defaultKeyFile);
    QByteArray decoded;
    decoded.clear();

    if (!certfile.exists()) {
        if (snakeoilCert.open(QFile::ReadOnly)) {
            decoded = QByteArray::fromBase64(snakeoilCert.readAll());
            snakeoilCert.close();
        }
        if (certfile.open(QFile::WriteOnly)) {
            certfile.write(decoded);
            certfile.close();
        }
    }

    if (!keyfile.exists()) {
        if (snakeoilKey.open(QFile::ReadOnly)) {
            decoded = QByteArray::fromBase64(snakeoilKey.readAll());
            snakeoilKey.close();
        }
        if (keyfile.open(QFile::WriteOnly)) {
            keyfile.write(decoded);
            keyfile.close();
        }
    }


    updateManager(QByteArray());

    //on_actionExport_Packets_JSON_triggered();

    QDEBUG() << "Load time:" <<  timeSinceLaunch();

    QDEBUG() << "Settings file loaded" << SETTINGSFILE;
    QDEBUG() << "Packets file loaded" << PACKETSFILE;
}


void themeTheButton(QPushButton * button)
{
    QPalette pal = button->palette();
    pal.setColor(QPalette::Button, QColor("#F5F5F5"));
    button->setAutoFillBackground(true);
    button->setPalette(pal);
    button->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    button->update();


}

void MainWindow::toggleUDPServer()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("udpServerEnable", !settings.value("udpServerEnable", true).toBool());
    applyNetworkSettings();
}
void MainWindow::toggleTCPServer()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("tcpServerEnable", !settings.value("tcpServerEnable", true).toBool());
    applyNetworkSettings();
}
void MainWindow::toggleSSLServer()
{
    QDEBUG();
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("sslServerEnable", !settings.value("sslServerEnable", true).toBool());
    applyNetworkSettings();
}

void MainWindow::generateConnectionMenu()
{


}
/*

{"githubpath":"https:\/\/api.github.com\/repos\/dannagle\/PacketSender\/releases\/7612134",
"windowsversion":"v5.4.2","macversion":"v5.4.2","linuxversion":"v5.4.2","windowsdownload":
"https:\/\/github.com\/dannagle\/PacketSender\/releases\/download\/v5.4.2\/PacketSender_5_4_2_2017-09-01.exe",
"windowsportable":"https:\/\/github.com\/dannagle\/PacketSender\/releases\/download\/v5.4.2\/PacketSenderPortable_5_4_2_2017-09-01.zip",
"macdownload":"https:\/\/github.com\/dannagle\/PacketSender\/releases\/download\/v5.4.2\/PacketSender_v5_4_2_2017-09-01.dmg","linuxdownload":
"https:\/\/github.com\/dannagle\/PacketSender\/releases\/download\/v5.4.2\/PacketSenderLinux_5_4_2_2017-09-01.AppImage"}


*/


void parserMajorMinorBuild(QString sw, unsigned int & major, unsigned int & minor, unsigned int & build)
{

    major = 0;
    minor = 0;
    build = 0;
    sw.replace("v", "");
    QStringList versionSplit = sw.split(".");

    if (versionSplit.size() >= 0) {
        major = versionSplit[0].toUInt();
    }

    if (versionSplit.size() >= 1) {
        minor = versionSplit[1].toUInt();
    }

    if (versionSplit.size() >= 2) {
        build = versionSplit[2].toUInt();
    }



}


void MainWindow::updateManager(QByteArray response)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    if (!response.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (!doc.isNull()) {
            //valid JSON
            QJsonObject json = doc.object();
            QString githubpath = json["githubpath"].toString();
            QString version = json["windowsversion"].toString();

#ifdef __APPLE__
            version = json["macversion"].toString();
#endif

#ifdef __linux__
            version = json["linuxversion"].toString();


#endif

            QString currentVersion = SW_VERSION;
            unsigned int majorCurrent, minorCurrent, buildCurrent;
            parserMajorMinorBuild(currentVersion, majorCurrent, minorCurrent, buildCurrent);

            //majorCurrent--;

            unsigned int majorNew, minorNew, buildNew;
            parserMajorMinorBuild(version, majorNew, minorNew, buildNew);

            bool needUpdate = false;
            if (majorNew > majorCurrent) {
                needUpdate = true;
            }

            if (majorNew == majorCurrent) {
                if (minorNew > minorCurrent) {
                    needUpdate = true;
                }

                if (minorNew == minorCurrent) {
                    if (buildNew > buildCurrent) {
                        needUpdate = true;
                    }
                }
            }

            QDEBUG() << "Current SW" << majorCurrent << minorCurrent << buildCurrent;
            QDEBUG() << "NEW SW"  << majorNew << minorNew << buildNew;

            if (needUpdate) {
                QDEBUG() << "Update is needed";
                QMessageBox msgBox;
                msgBox.setWindowIcon(QIcon(":pslogo.png"));
                msgBox.setWindowTitle("Updates.");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText("There is a new Packet Sender available.\n\nDownload?");
                int yesno = msgBox.exec();
                if (yesno == QMessageBox::Yes) {
                    QDesktopServices::openUrl(QUrl("https://packetsender.com/download"));
                } else {
                    QDEBUG() << "Skip a few checks";
                    QDateTime next = QDateTime::currentDateTime().addDays(14);
                    settings.setValue("updateLastChecked", next.toString(FULLDATETIMEFORMAT));
                }
            } else {
                QDEBUG() << "SW is up to date";
            }


        }

        return;
    }

    if(! (SNAPBUILD)) {
        //snaps do not need to check for updates.

        if (!settings.value("checkforUpdatesAsked", false).toBool()) {
            settings.setValue("checkforUpdatesAsked", true);
            QMessageBox msgBox;
            msgBox.setWindowIcon(QIcon(":pslogo.png"));
            msgBox.setWindowTitle("Updates.");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Let Packet Sender check for updates weekly?");
            int yesno = msgBox.exec();
            if (yesno == QMessageBox::Yes) {
                QDEBUG() << "Will check for updates";
                settings.setValue("checkforUpdates", true);
            } else {
                QDEBUG() << "Will NOT check for updates";
                settings.setValue("checkforUpdates", false);
            }

            settings.sync();
        }
    }



    if (settings.value("checkforUpdates", true).toBool()) {
        QString updateLastCheckedString = settings.value("updateLastChecked").toString();
        QDateTime updateLastChecked = QDateTime::fromString(updateLastCheckedString, FULLDATETIMEFORMAT);
        QDateTime now = QDateTime::currentDateTime();
        QDateTime next = updateLastChecked.addDays(DAYS_BETWEEN_UPDATES);

        if (next < now) {
            QDEBUG() << "Time to check for updates" << UPDATE_URL;
            http->get(QNetworkRequest(QUrl(UPDATE_URL)));
            settings.setValue("updateLastChecked", now.toString(FULLDATETIMEFORMAT));
        } else {
            QDEBUG() << "Next update check will be" << next.toString(FULLDATETIMEFORMAT);
        }
    }
}


QPushButton * MainWindow::generatePSLink()
{
    QPushButton * returnButton = new QPushButton("PacketSender.com");
    returnButton->setStyleSheet(HYPERLINKSTYLE);
    returnButton->setIcon(QIcon(":pslogo.png"));
    returnButton->setFlat(true);
    returnButton->setCursor(Qt::PointingHandCursor);
    connect(returnButton, SIGNAL(clicked()),
            this, SLOT(gotoPacketSenderDotCom()));

    return returnButton;

}

void MainWindow::toTrafficLog(Packet logPacket)
{

    static bool initialpackets = false;

    if (ui->logTrafficCheck->isChecked()) {
        if ((!logPacket.toIP.isEmpty() && !logPacket.fromIP.isEmpty())
                || (!logPacket.errorString.isEmpty())
           ) {
            packetsLogged.prepend(logPacket);
        } else {
            QDEBUG() << "Discarded packet";
        }
    }

    int trafficlogsize = packetsLogged.size();
    ui->trafficLogClearButton->setText("Clear Log ("+QString::number(trafficlogsize)+")");


    if(!initialpackets) {

        ui->trafficLogTable->resizeColumnsToContents();
        ui->trafficLogTable->resizeRowsToContents();
        ui->trafficLogTable->horizontalHeader()->setStretchLastSection(true);

        initialpackets = true;

    }


}


void MainWindow::UDPServerStatus()
{

    if (packetNetwork.UDPListening()) {
        QString ports = packetNetwork.getUDPPortString();
        int portcount = packetNetwork.getUDPPortsBound().size();
        udpServerStatus->setToolTip(ports);
        if(portcount > 3) {
            udpServerStatus->setText("UDP: " + QString::number(portcount) + " Ports");
        } else {
            udpServerStatus->setText("UDP:" + ports);
        }

    } else {
        udpServerStatus->setText("UDP Server Disabled");

    }

    //updatewidget


}

void MainWindow::SSLServerStatus()
{
    if (packetNetwork.SSLListening()) {
        QString ports = packetNetwork.getSSLPortString();
        int portcount = packetNetwork.getSSLPortsBound().size();
        sslServerStatus->setToolTip(ports);
        if(portcount > 3) {
            sslServerStatus->setText("SSL: " + QString::number(portcount) + " Ports");
        } else {
            sslServerStatus->setText("SSL:" + ports);
        }

    } else {
        sslServerStatus->setText("SSL Server Disabled");

    }


}


void MainWindow::TCPServerStatus()
{
    if (packetNetwork.TCPListening()) {
        QString ports = packetNetwork.getTCPPortString();
        int portcount = packetNetwork.getTCPPortsBound().size();
        tcpServerStatus->setToolTip(ports);
        if(portcount > 3) {
            tcpServerStatus->setText("TCP: " + QString::number(portcount) + " Ports");
        } else {
            tcpServerStatus->setText("TCP:" + ports);
        }


    } else {
        tcpServerStatus->setText("TCP Server Disabled");

    }


}

QPushButton *MainWindow::generateDNLink()
{

    QPushButton * returnButton = new QPushButton("DanNagle.com");
    returnButton->setStyleSheet(HYPERLINKSTYLE);
    returnButton->setIcon(QIcon(":dannagle_logo.png"));
    returnButton->setFlat(true);
    returnButton->setCursor(Qt::PointingHandCursor);
    connect(returnButton, SIGNAL(clicked()),
            this, SLOT(gotoDanNagleDotCom()));

    return returnButton;

}


void MainWindow::loadPacketsTable()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    tableActive = false;
    Packet tempPacket;


    QList<Packet> packetsSavedFiltered;
    packetsSavedFiltered.clear();

    QString filterString = ui->searchLineEdit->text().toLower().trimmed();


    foreach (tempPacket, packetsSaved) {

        if (tempPacket.name.toLower().contains(filterString) ||
                tempPacket.hexToASCII(tempPacket.hexString).toLower().contains(filterString) ||
                tempPacket.toIP.toLower().contains(filterString) ||
                QString::number(tempPacket.port).contains(filterString)
           ) {
            packetsSavedFiltered.append(tempPacket);
            continue;
        }
    }

    QDEBUGVAR(packetsSavedFiltered.size());



    ui->packetsTable->clear();



    packetSavedTableHeaders  = Settings::defaultPacketTableHeader();
    packetSavedTableHeaders = settings.value("packetSavedTableHeaders", packetSavedTableHeaders).toStringList();

    ui->packetsTable->setColumnCount(packetSavedTableHeaders.size());

    ui->packetsTable->verticalHeader()->show();
    ui->packetsTable->horizontalHeader()->show();
    ui->packetsTable->setHorizontalHeaderLabels(packetSavedTableHeaders);
    if (packetsSavedFiltered.isEmpty()) {
        ui->packetsTable->setRowCount(0);
    } else {
        ui->packetsTable->setRowCount(packetsSavedFiltered.count());
    }

    unsigned int rowCounter = 0;
    foreach (tempPacket, packetsSavedFiltered) {
        populateTableRow(rowCounter, tempPacket);
        rowCounter++;
    }



    ui->packetsTable->resizeColumnsToContents();
    ui->packetsTable->resizeRowsToContents();
    ui->packetsTable->horizontalHeader()->setStretchLastSection(true);

    tableActive = true;


}

void MainWindow::httpFinished(QNetworkReply* pReply)
{

    QByteArray data = pReply->readAll();
    QString str = QString(data);
    str.truncate(1000);
    QDEBUG() << "finished http." << str;
    if (str.contains("windowsversion")
            && str.contains("macversion")
       ) {
        //Received valid update data.
        QDEBUG() << "Valid update string";
        updateManager(data);
    } else {
        QDEBUG() << "Did not receive a valid update string";
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

quint64 MainWindow::timeSinceLaunch()
{
    static QTime launchTimer = QTime::currentTime();
    if (launchTimer.isValid()) {
        return launchTimer.elapsed();

    } else {
        launchTimer.start();
        return 0;
    }

}

void MainWindow::on_packetHexEdit_lostFocus()
{

    QString quicktestHex =  ui->packetHexEdit->text();

    ui->packetASCIIEdit->setText(Packet::hexToASCII(quicktestHex));
    ui->packetASCIIEdit->setToolTip("");
    ui->packetHexEdit->setText(quicktestHex);


}

void MainWindow::on_packetASCIIEdit_lostFocus()
{

    QString quicktestASCII =  ui->packetASCIIEdit->text();
    ui->packetHexEdit->setText(Packet::ASCIITohex(quicktestASCII));


    QString quicktestASCII2 =  ui->packetHexEdit->text();

    ui->packetASCIIEdit->setText(Packet::hexToASCII(quicktestASCII2));
    ui->packetASCIIEdit->setToolTip("");

    qDebug() << __FILE__ << "/" << __LINE__ << "on_serialASCIIEdit_lostFocus";

}

void MainWindow::statusBarMessage(const QString &msg, int timeout = 3000, bool override = false)
{
    QString currentMsg = statusBar()->currentMessage();

    if (currentMsg.size() > 10) {
        override = true;
    }
    if (currentMsg.size() > 0 && !override) {
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
    static QStringList noMCastList;
    Packet toSend;


    foreach (toSend, packetsSaved) {
        if (toSend.name == packetName) {

            if (PacketNetwork::isMulticast(toSend.toIP) && (!noMCastList.contains(toSend.toIP))) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Multicast detected.");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Do you wish to join the multicast group?");
                int yesno = msgBox.exec();
                if (yesno == QMessageBox::No) {
                    noMCastList.append(toSend.toIP);
                } else {
                    on_actionJoin_IPv4_triggered(toSend.toIP);
                }

            }

            if (toSend.toIP.trimmed() == "255.255.255.255") {

                QSettings settings(SETTINGSFILE, QSettings::IniFormat);
                bool sendResponse = settings.value("sendReponse", false).toBool();
                if (sendResponse) {
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Broadcast with responses!");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::No);
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText("You are sending a broadcast packet with responses enabled.\n\nThis could cause traffic flooding. Continue?");
                    int yesno = msgBox.exec();
                    if (yesno == QMessageBox::No) {
                        return;
                    }

                }
            }

            if (toSend.repeat > 0) {
                toSend.timestamp = QDateTime::currentDateTime();

                stopResendingButton->setStyleSheet("QPushButton { color: green; } QPushButton::hover { color: #BC810C; } ");
                packetsRepeat.append(toSend);
                stopResendingButton->setText("Resending (" + QString::number(packetsRepeat.size()) + ")");
                stopResending = 0;
            }

            lastSendPacket = toSend;
            QByteArray sendData = toSend.getByteArray();
            statusBarMessage("Send: " + packetName + "  (" + QString::number(sendData.size()) + " bytes)");
            emit sendPacket(toSend);
        }
    }

}

void MainWindow::on_savePacketButton_clicked()
{

    Packet testPacket;
    testPacket.init();
    testPacket.name = ui->packetNameEdit->text().trimmed();
    if (testPacket.name.isEmpty()) {

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
    testPacket.repeat = Packet::oneDecimal(ui->resendEdit->text().toFloat());

    testPacket.saveToDB();
    packetsSaved = Packet::fetchAllfromDB("");

    //ui->searchLineEdit->setText("");
    loadPacketsTable();

}

void MainWindow::saveSession(Packet sessionPacket)
{

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    //settings.setValue("packetNameEditSession", ui->packetNameEdit->text());
    settings.setValue("packetIPEditSession", ui->packetIPEdit->text());
    settings.setValue("packetHexEditSession", ui->packetHexEdit->text());
    settings.setValue("udptcpComboBoxSession", ui->udptcpComboBox->currentText());
    settings.setValue("packetPortEditSession", ui->packetPortEdit->text());
    settings.setValue("resendEditSession", ui->resendEdit->text());

}


void MainWindow::on_testPacketButton_clicked()
{
    Packet testPacket;
    static QStringList noMCastList;
    testPacket.init();

    if (ui->udptcpComboBox->currentText() == "SSL") {

        if (!QSslSocket::supportsSsl()) {

            QMessageBox msgBox;
            msgBox.setText("This computer does not support SSL.\n\nExpected SSL:" + QSslSocket::sslLibraryVersionString());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle("No SSL Library.");
            msgBox.exec();
            ui->udptcpComboBox->setFocus();
            return;

        }
    }

    if (ui->testPacketButton->text().contains("Multi")) {
        QDEBUG() << "We are multi";
        int packetCount = 0;

        QList<QTableWidgetItem *> totalSelected = ui->packetsTable->selectedItems();
        if (!totalSelected.isEmpty()) {
            QTableWidgetItem * item;
            QList<int> usedRows;
            usedRows.clear();
            foreach (item, totalSelected) {
                if (usedRows.contains(item->row())) {
                    continue;
                } else {
                    usedRows.append(item->row());
                    Packet clickedPacket = Packet::fetchTableWidgetItemData(item);

                    emit sendPacket(clickedPacket);
                    packetCount++;

                }

            }

        }
        statusBarMessage("Sending " + QString::number(packetCount) + " packets");
        return;

    }


    testPacket.name = ui->packetNameEdit->text().trimmed();
    testPacket.toIP = ui->packetIPEdit->text().trimmed();
    testPacket.hexString =  ui->packetHexEdit->text().simplified();
    testPacket.tcpOrUdp = ui->udptcpComboBox->currentText();
    testPacket.sendResponse =  0;
    testPacket.port = ui->packetPortEdit->text().toUInt();
    testPacket.repeat = Packet::oneDecimal(ui->resendEdit->text().toFloat());

    //Save Session!
    saveSession(testPacket);

    if (testPacket.toIP.isEmpty()) {

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


    if (testPacket.port == 0) {

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



    bool isIPv6 = testPacket.toIP.contains(":");

    if(isIPv6 && (!packetNetwork.IPv6Enabled())) {

        QMessageBox msgBox;
        msgBox.setWindowTitle("IPv6?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Enable IPv6 support?");
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::Yes) {
            packetNetwork.setIPmode(8); //both 4 and 6
            packetNetwork.kill();
            packetNetwork.init();
        }
    }


    if (PacketNetwork::isMulticast(testPacket.toIP) && (!noMCastList.contains(testPacket.toIP))) {

        //are we joined?



        if(!packetNetwork.canSendMulticast(testPacket.toIP)) {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Multicast detected.");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Join UDP multicast group?");

            int yesno = msgBox.exec();
            if (yesno == QMessageBox::Yes) {
                on_actionJoin_IPv4_triggered(testPacket.toIP);
            } else {
                noMCastList.append(testPacket.toIP);
            }
        }
    }

    if (testPacket.toIP.trimmed() == "255.255.255.255") {

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        bool sendResponse = settings.value("sendReponse", false).toBool();
        if (sendResponse) {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Broadcast with responses!");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("You are sending a broadcast packet with responses enabled.\n\nThis could cause traffic flooding. Continue?");
            int yesno = msgBox.exec();
            if (yesno == QMessageBox::No) {
                return;
            }
        }
    }


    lastSendPacket = testPacket;
    lastSendPacket.name.clear();

    if (testPacket.repeat > 0) {
        if((testPacket.isTCP() || (testPacket.isSSL()))  && ui->persistentTCPCheck->isChecked()) {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Resend TCP with persistent connections!");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("You are resending a TCP packet with presistent connecitons. The UI could spawn numerous windows! \n\nUncheck persistent connection? (Recommended)");
            int yesno = msgBox.exec();
            if(yesno == QMessageBox::Yes) {
                ui->persistentTCPCheck->setChecked(false);
                on_persistentTCPCheck_clicked(false);
            }
            if(yesno == QMessageBox::Cancel) {
                return;
            }

        }

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

    if (indexes.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setText("No packets selected.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("Select a packet.");
        msgBox.exec();
        return;
    }

    QStringList nameList;
    nameList.clear();

    foreach (index, indexes) {
        selected = index.data(Packet::PACKET_NAME).toString();
        if (!nameList.contains(selected)) {
            nameList.append(selected);
        }
    }

    if (!nameList.isEmpty()) {
        Packet::removeFromDBList(nameList);
    }

    packetsSaved = Packet::fetchAllfromDB("");
    ui->searchLineEdit->setText("");
    loadPacketsTable();

}



void MainWindow::on_packetIPEdit_lostFocus()
{
    QString ipPacket = ui->packetIPEdit->text().trimmed();
    QHostAddress address(ipPacket);
    if (QAbstractSocket::IPv4Protocol == address.protocol()) {
        QDEBUG() << "Valid IPv4 address.";
    } else if (QAbstractSocket::IPv6Protocol == address.protocol()) {
        QDEBUG() << "Valid IPv6 address.";
    } else {
        QHostInfo info = QHostInfo::fromName(ipPacket);
        if (info.error() != QHostInfo::NoError) {
            ui->packetIPEdit->setText("");
            ui->packetIPEdit->setPlaceholderText("Invalid Address / DNS failed");
        } else {

            QSettings settings(SETTINGSFILE, QSettings::IniFormat);

            if (settings.value("resolveDNSOnInputCheck", false).toBool()) {
                ui->packetIPEdit->setText(info.addresses().at(0).toString());
            } else {
                statusBarMessage(ipPacket + " --> " + info.addresses().at(0).toString());
            }
        }
    }
}

void MainWindow::on_packetPortEdit_lostFocus()
{
    QString portPacket = ui->packetPortEdit->text().trimmed();
    unsigned int port = 0 ;
    bool ok;
    port = portPacket.toUInt(&ok, 0);
    QDEBUGVAR(port);
    if (port <= 0) {
        ui->packetPortEdit->setText("");
        ui->packetPortEdit->setPlaceholderText("Invalid Port");
    } else {
        ui->packetPortEdit->setText(QString::number(port));
    }
}

//   QDEBUG() << "cell changed";

void MainWindow::removePacketfromMemory(Packet thepacket)
{
    for (int i = 0; i < packetsSaved.size(); i++) {
        if (thepacket.name == packetsSaved[i].name) {
            packetsSaved.removeAt(i);
            break;
        }

    }

}

void MainWindow::on_packetsTable_itemChanged(QTableWidgetItem *item)
{
    if (!tableActive) {
        return;
    }
    tableActive =  false;
    QString datatype = item->data(Packet::DATATYPE).toString();
    QString newText = item->text();
    QDEBUG() << "cell changed:" << datatype << item->text();
    int fullrefresh = 0;

    Packet updatePacket = Packet::fetchTableWidgetItemData(item);
    if (datatype == "name") {
        Packet::removeFromDB(updatePacket.name); //remove old before inserting new.
        removePacketfromMemory(updatePacket);
        updatePacket.name = newText;
        fullrefresh = 1;
    }
    if (datatype == "toIP") {
        QHostAddress address(newText);
        if (QAbstractSocket::IPv4Protocol == address.protocol()) {
            updatePacket.toIP = newText;
        }        else if (QAbstractSocket::IPv6Protocol == address.protocol()) {
            updatePacket.toIP = newText;
        } else {
            QHostInfo info = QHostInfo::fromName(newText);
            if (info.error() == QHostInfo::NoError) {

                updatePacket.toIP = newText;

                QSettings settings(SETTINGSFILE, QSettings::IniFormat);

                if (settings.value("resolveDNSOnInputCheck", false).toBool()) {
                    updatePacket.toIP = (info.addresses().at(0).toString());
                } else {
                    statusBarMessage(newText + " --> " + info.addresses().at(0).toString());
                }
            }

        }


    }
    if (datatype == "port") {
        int portNum = newText.toUInt();
        if (portNum > 0) {
            updatePacket.port = portNum;
        }
    }
    if (datatype == "repeat") {
        float repeat = Packet::oneDecimal(newText.toFloat());
        updatePacket.repeat = repeat;
    }
    if (datatype == "tcpOrUdp") {
        if ((newText.trimmed().toUpper() == "TCP") || (newText.trimmed().toUpper() == "UDP") || (newText.trimmed().toUpper() == "SSL")) {
            updatePacket.tcpOrUdp = newText.trimmed().toUpper();
        }
    }
    if (datatype == "ascii") {
        QString hex = Packet::ASCIITohex(newText);
        updatePacket.hexString = hex;
    }
    if (datatype == "hexString") {
        QString hex = newText;
        QString ascii = Packet::hexToASCII(newText);
        updatePacket.hexString = newText;
    }

    updatePacket.saveToDB();
    packetsSaved = Packet::fetchAllfromDB("");

    if (fullrefresh) {
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

void MainWindow::shortcutkey1()
{
    ui->packetNameEdit->setFocus();
}
void MainWindow::shortcutkey2()
{
    ui->packetASCIIEdit->setFocus();
}
void MainWindow::shortcutkey3()
{
    ui->packetHexEdit->setFocus();
}
void MainWindow::shortcutkey4()
{
    ui->packetIPEdit->setFocus();
}
void MainWindow::shortcutkey5()
{
    ui->packetPortEdit->setFocus();
}
void MainWindow::shortcutkey6()
{
    ui->resendEdit->setFocus();
}
void MainWindow::shortcutkey7()
{
    ui->udptcpComboBox->setFocus();
}


//packetSavedTableHeaders <<"Send" <<"Resend (s)"<< "Name" << "To Address" << "To Port" << "Method" << "ASCII" << "Hex";


int MainWindow::findColumnIndex(QListWidget * lw, QString search)
{
    QListWidgetItem * item;
    QString text;
    int size = lw->count();

    for (int i = 0; i < size; i++) {
        item = lw->item(i);
        text = item->text();
        if (text == search) {
            return i;
        }

    }
    QDEBUGVAR(search);
    return -1;

}

void MainWindow::populateTableRow(int rowCounter, Packet tempPacket)
{
    QTableWidgetItem * tItem;

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


    ui->packetsTable->setCellWidget(rowCounter, packetSavedTableHeaders.indexOf("Send"), sendButton);

    tItem = new QTableWidgetItem(QString::number(tempPacket.repeat));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "repeat");
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("Resend (sec)"), tItem);
    //QDEBUGVAR(tempPacket.name);

    tItem = new QTableWidgetItem(tempPacket.name);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "name");
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("Name"), tItem);
    //QDEBUGVAR(tempPacket.name);

    tItem = new QTableWidgetItem(tempPacket.toIP);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "toIP");

    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("To Address"), tItem);
    //QDEBUGVAR(tempPacket.toIP);

    tItem = new QTableWidgetItem(QString::number(tempPacket.port));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "port");
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("To Port"), tItem);
    // QDEBUGVAR(tempPacket.port);

    tItem = new QTableWidgetItem(tempPacket.tcpOrUdp);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "tcpOrUdp");
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("Method"), tItem);
    // QDEBUGVAR(tempPacket.tcpOrUdp);

    tItem = new QTableWidgetItem(tempPacket.hexToASCII(tempPacket.hexString));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "ascii");

    QSize tSize = tItem->sizeHint();
    tSize.setWidth(200);
    tItem->setSizeHint(tSize);

    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("ASCII"), tItem);
    //QDEBUGVAR(tempPacket.hexString);

    tItem = new QTableWidgetItem(tempPacket.hexString);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, "hexString");
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf("Hex"), tItem);
    //QDEBUGVAR(tempPacket.hexString);
}

void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (deleteWidgets) {
            if (QWidget* widget = item->widget())
                delete widget;
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void MainWindow::packetTable_checkMultiSelected()
{

    //how many are selected?
    QTableWidgetItem * checkItem;
    QList<Packet> packetList;
    Packet clickedPacket;
    QList<QTableWidgetItem *> totalSelected = ui->packetsTable->selectedItems();
    packetList.clear();
    QStringList buttonsList;
    buttonsList.clear();


    foreach (checkItem, totalSelected) {
        clickedPacket = Packet::fetchTableWidgetItemData(checkItem);
        if (buttonsList.contains(clickedPacket.name)) {
            continue;
        } else {
            packetList.append(clickedPacket);
            buttonsList.append(clickedPacket.name);
        }
    }


    if (packetList.size() >= 2) {
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


    if (item->isSelected()) {
        Packet clickedPacket = Packet::fetchTableWidgetItemData(item);

        if (item->column() != 0) {
            ui->packetNameEdit->setText(clickedPacket.name);
            ui->packetHexEdit->setText(clickedPacket.hexString);
            ui->packetIPEdit->setText(clickedPacket.toIP);
            ui->packetPortEdit->setText(QString::number(clickedPacket.port));
            ui->resendEdit->setText(QString::number(clickedPacket.repeat));
            ui->udptcpComboBox->setCurrentIndex(ui->udptcpComboBox->findText(clickedPacket.tcpOrUdp));
            ui->packetASCIIEdit->setText(Packet::hexToASCII(clickedPacket.hexString));
            ui->packetASCIIEdit->setToolTip("");

        }
    }
}


void MainWindow::slowRefreshTimerTimeout()
{
    QString oldtitle = this->windowTitle();
    QString titleString = "Packet Sender - IPs: ";
    QTextStream out(&titleString);

    //QDEBUGVAR(packetsLogged.size());
    //QDEBUGVAR(packetsLogged.rowCount());


    QNetworkAddressEntry entry;
    QList<QNetworkAddressEntry> allEntries = SubnetCalc::nonLoopBackAddresses();

    for (int i=0; i < allEntries.size(); i++) {
        entry = allEntries[i];
        out << entry.ip().toString();
        if(i < allEntries.size() - 1) {
            out <<", ";
        }
    }

    if(titleString != oldtitle) {
        setWindowTitle(titleString);
    }


    //In case the multicast switch had a problem, rejoin all groups.
    packetNetwork.reJoinMulticast();

}

void MainWindow::refreshTimerTimeout()
{

    QDateTime now = QDateTime::currentDateTime();

    for (int i = 0; i < packetsRepeat.size() && !stopResending; i++) {

        int repeatMS = (int)(packetsRepeat[i].repeat * 1000 - 100);

        if (packetsRepeat[i].timestamp.addMSecs(repeatMS) < now) {
            packetsRepeat[i].timestamp = now;

            if (((resendCounter + 1) < cancelResendNum) || (cancelResendNum == 0)) {

                emit sendPacket(packetsRepeat[i]);
                statusBarMessage("Send: " + packetsRepeat[i].name + " (Resend)");
                resendCounter++;

            } else {
                stopResending = 1;
            }

        }
    }


    if (packetsRepeat.isEmpty() || stopResending) {
        stopResendingButton->hide();
        packetsRepeat.clear();
        stopResending = 0;
        resendCounter = 0;

    } else {
        stopResendingButton->show();
    }


    while (maxLogSize > 0 && packetsLogged.size() > maxLogSize) {
        packetsLogged.removeFirst();
    }


        //ui->mainTabWidget->setTabText(1,"Traffic Log (" + QString::number(packetsLogged.size()) +")");

    //got nothing else to do. check datagrams.
    packetNetwork.readPendingDatagrams();


}

void MainWindow::on_trafficLogClearButton_clicked()
{
    packetsLogged.clear();
    Packet tempPacket;
    ui->trafficLogClearButton->setText("Clear Log (0)");
}


void MainWindow::on_saveTrafficPacket_clicked()
{
    QModelIndexList indexes = ui->trafficLogTable->selectionModel()->selectedIndexes();
    QModelIndex index;
    QDEBUG() << "Save traffic packets";
    QString selected;


    ;
    QString namePrompt;
    bool ok;
    foreach (index, indexes) {
        Packet savePacket = packetsLogged.getPacket(index);
        QDEBUG() << "Saving" << savePacket.name;
        namePrompt = QInputDialog::getText(this, tr("Save Packet"),
                                           tr("Packet name:"), QLineEdit::Normal,
                                           savePacket.name, &ok);
        if (ok && !namePrompt.isEmpty()) {
            savePacket.name = namePrompt.trimmed();
        }


        if (savePacket.toIP.toUpper().trimmed() == "YOU") {
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
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    packetsRepeat.clear();

    multiSendDelay = settings.value("multiSendDelay", 0).toFloat();
    cancelResendNum = settings.value("cancelResendNum", 0).toInt();
    resendCounter = 0;

    int joinedSize = packetNetwork.multicastStringList().size();
    if(joinedSize > 0) {
        statusBarMessage("Left " + QString::number(joinedSize) + " multicast group(s)");
    }

    packetNetwork.kill();
    packetNetwork.init();
    packetNetwork.responseData = settings.value("responseHex", "").toString().trimmed();
    packetNetwork.sendResponse = settings.value("sendReponse", false).toBool();
    ui->persistentTCPCheck->setChecked(settings.value("persistentTCPCheck", false).toBool());
    on_persistentTCPCheck_clicked(ui->persistentTCPCheck->isChecked());

    UDPServerStatus();
    TCPServerStatus();
    SSLServerStatus();

}

void MainWindow::cancelResends()
{
    stopResendingButton->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    stopResendingButton->setText("Resending");
    stopResending = 1;
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



void MainWindow::on_searchLineEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

    loadPacketsTable();

}


void MainWindow::on_toClipboardButton_clicked()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);


    QClipboard *clipboard = QApplication::clipboard();
    QModelIndexList indexes = ui->trafficLogTable->selectionModel()->selectedIndexes();
    QModelIndex index;
    QDEBUG() << "Save traffic packets";
    QString selected;


    ;
    QString clipString;
    clipString.clear();
    QTextStream out;
    out.setString(&clipString);

    if (indexes.size() == 0) {

        QMessageBox msgBox;
        msgBox.setText("No packets selected.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("Clipboard unchanged.");
        msgBox.exec();
        return;
    }
    foreach (index, indexes) {
        Packet savePacket = packetsLogged.getPacket(index);
        out << "Time: " << savePacket.timestamp.toString(DATETIMEFORMAT) << "\n";
        out << "TO: " << savePacket.toIP << ":" << savePacket.port << "\n";
        out << "From: " << savePacket.fromIP << ":" << savePacket.fromPort << "\n";
        out << "Method: " << savePacket.tcpOrUdp << "\n";
        out << "Error: " << savePacket.errorString;
        out << "\n\nASCII:" << "\n";
        out << savePacket.hexToASCII(savePacket.hexString) << "\n";
        out << "\n\nHEX:" << "\n";
        out << savePacket.hexString << "\n";

        break;

    }

    QMessageBox msgBox;

    if (settings.value("copyUnformattedCheck", true).toBool()) {
        Packet savePacket = packetsLogged.getPacket(indexes.first());
        clipboard->setText(QString(savePacket.getByteArray()));
        msgBox.setText("Raw packet data sent to your clipboard. \nChange the settings if you prefer translated data.");
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



void MainWindow::on_packetsTable_itemSelectionChanged()
{
    packetTable_checkMultiSelected();

}


void MainWindow::on_bugsLinkButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://bugtracker.dannagle.com/"));
}


void MainWindow::on_forumsPacketSenderButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://forums.packetsender.com/"));


}


void MainWindow::on_saveLogButton_clicked()
{
    static QString fileName = QDir::homePath() + QString("/trafficlog.log");

    fileName = QFileDialog::getSaveFileName(this, tr("Save Traffic Log"),
                                            QDir::toNativeSeparators(fileName), tr("log (*.log)"));

    QStringList testExt = fileName.split(".");
    QString ext = "";
    if (testExt.size() > 0) {
        ext = testExt.last();
    }
    if (ext != "log") {
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
    foreach (tempPacket, packetsLogged.list()) {

        exportString.append(tempPacket.timestamp.toString(DATETIMEFORMAT));
        exportString.append(delim);
        exportString.append(tempPacket.fromIP);
        exportString.append(delim);
        exportString.append(QString::number(tempPacket.fromPort));
        exportString.append(delim);
        exportString.append(tempPacket.toIP);
        exportString.append(delim);
        exportString.append(QString::number(tempPacket.port));
        exportString.append(delim);
        exportString.append(tempPacket.tcpOrUdp);
        exportString.append(delim);
        exportString.append(tempPacket.errorString);
        exportString.append(delim);
        exportString.append(tempPacket.hexToASCII(tempPacket.hexString));
        exportString.append(delim);
        exportString.append(tempPacket.hexString);
        exportString.append(delim);
        exportString.append("\n");
    }


    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        file.write(exportString.toLatin1());
        file.close();
    }

    statusBarMessage("Save Log: " + fileName);

}

void MainWindow::setIPMode()
{

    bool isIPv6 = packetNetwork.IPv6Enabled();

    IPmodeButton->setText(packetNetwork.getIPmode());

    if (isIPv6) {
        QDEBUG() << "Set IPv6 stylesheet";
        IPmodeButton->setStyleSheet(IPv6Stylesheet);
    } else {
        IPmodeButton->setStyleSheet(IPv4Stylesheet);
    }


}


void MainWindow::toggleIPv4_IPv6()
{
    QString currentMode = IPmodeButton->text();
    if(currentMode.contains("v4") || currentMode.contains(".")) {
        packetNetwork.setIPmode(6);
    } else {
        packetNetwork.setIPmode(4);
    }

    setIPMode();

    applyNetworkSettings();


}


void MainWindow::on_actionAndroid_App_triggered()
{
    QDesktopServices::openUrl(QUrl("https://packetsender.com/android"));
}


void MainWindow::on_actioniOS_App_triggered()
{
    QDesktopServices::openUrl(QUrl("https://packetsender.com/ios"));
}


void MainWindow::on_actionForums_triggered()
{
    QDesktopServices::openUrl(QUrl("https://forums.naglecode.com/"));
}

void MainWindow::on_actionFollow_NagleCode_triggered()
{
    QDesktopServices::openUrl(QUrl("https://packetsender.com/twitter"));
}

void MainWindow::on_actionConnect_on_LinkedIn_triggered()
{
    QDesktopServices::openUrl(QUrl("https://packetsender.com/linkedin"));
}



void MainWindow::on_actionAbout_triggered()
{
    About * about = new About(this);
    about->show();
}

void MainWindow::on_actionJoin_IPv4_triggered(QString address)
{
    MulticastSetup mcast(&packetNetwork, this);
    if((!address.isEmpty())) {
        mcast.setIP(address);
    }
    mcast.exec();
    UDPServerStatus();
    QDEBUG();
}

void MainWindow::on_actionHelp_triggered()
{
    //Open URL in browser
    QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation"));
}

void MainWindow::on_actionSettings_triggered()
{
    Settings settings;
    int accepted = settings.exec();
    if (accepted) {
        setIPMode();
        applyNetworkSettings();
        loadPacketsTable();
    }
}

void MainWindow::on_actionExit_triggered()
{
    QDEBUG();
    exit(0);
}

void MainWindow::on_actionImport_Packets_JSON_triggered()
{

    static QString fileName = QDir::homePath() + QString("/packets.json");


    fileName = QFileDialog::getOpenFileName(this, tr("Import JSON"),
                                            fileName,
                                            tr("JSON db (*.json)"));

    QDEBUGVAR(fileName);

    if (fileName.isEmpty()) {
        return;
    }

    QByteArray packetsjson;
    QFile jsonFile(fileName);
    if (jsonFile.open(QFile::ReadOnly)) {
        packetsjson = jsonFile.readAll();
        jsonFile.close();
    }

    QList<Packet> importList = Packet::ImportJSON(packetsjson);
    Packet importPacket;

    foreach (importPacket, importList) {
        QDEBUGVAR(importPacket.name);
    }

    if (!importList.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Found " + QString::number(importList.size()) + " packets!");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Import " + QString::number(importList.size()) + " packets?\n\nPacket Sender will overwrite packets with the same name.");
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::No) {
            statusBarMessage("Import Cancelled");
            return;
        } else {
            foreach (importPacket, importList) {
                importPacket.saveToDB();

            }
            packetsSaved = Packet::fetchAllfromDB("");
            statusBarMessage("Import Finished");
            loadPacketsTable();
        }

    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Not a database");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Found no packets in this file. It may not be a Packet Sender export");
        msgBox.exec();
        return;
        statusBarMessage("Import Cancelled");
    }

    return;



}

void MainWindow::on_actionExport_Packets_JSON_triggered()
{

    static QString fileName = QDir::homePath() + "/packets.json";

    fileName = QFileDialog::getSaveFileName(this, tr("Save JSON"),
                                            QDir::toNativeSeparators(fileName), tr("JSON db (*.json)"));

    if (fileName.isEmpty()) {
        return;
    }

    QStringList testExt = fileName.split(".");
    QString ext = "";
    if (testExt.size() > 0) {
        ext = testExt.last();
    }
    if (ext != "json") {
        fileName.append(".json");
    }
    QDEBUGVAR(fileName);

    QFile jsonFile(fileName);

    if (jsonFile.open(QFile::WriteOnly)) {
        jsonFile.write(Packet::ExportJSON(Packet::fetchAllfromDB("")));
        jsonFile.close();
        statusBarMessage("Export: " + fileName);
    } else {

        QMessageBox msgBox;
        msgBox.setWindowTitle("Could not save");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Could not open " + fileName + " for saving.");
        msgBox.exec();
        return;

    }


}

void MainWindow::on_actionCloud_triggered()
{
    QDEBUG();
    CloudUI * cloudUI = new CloudUI(this);
    if (!connect(cloudUI, &CloudUI::packetsImported, this, &MainWindow::packetsImported)) {
        QDEBUG() << "cloudUI packetsImported connection false";
    }
    cloudUI->exec();

}


void MainWindow::packetsImported(QList<Packet> packetSet)
{

    if (packetSet.size() > 0) {

        Packet importPacket;
        QDEBUGVAR(packetSet.size());
        foreach (importPacket, packetSet) {
            importPacket.saveToDB();
        }
        packetsSaved = Packet::fetchAllfromDB("");
        statusBarMessage("Import Finished");
        loadPacketsTable();

    }

}


void MainWindow::on_actionImport_Packets_triggered()
{

    static QString fileName = QDir::homePath() + QString("/packetsender_export.ini");


    fileName = QFileDialog::getOpenFileName(this, tr("Import File"),
                                            fileName,
                                            tr("INI db (*.ini)"));

    QDEBUGVAR(fileName);

    if (fileName.isEmpty()) {
        return;
    }

    QList<Packet> importList = Packet::fetchAllfromDB(fileName);
    Packet importPacket;

    foreach (importPacket, importList) {
        QDEBUGVAR(importPacket.name);
    }

    if (!importList.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Found " + QString::number(importList.size()) + " packets!");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Import " + QString::number(importList.size()) + " packets?\n\nPacket Sender will overwrite packets with the same name.");
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::No) {
            statusBarMessage("Import Cancelled");
            return;
        } else {
            foreach (importPacket, importList) {
                importPacket.saveToDB();

            }
            packetsSaved = Packet::fetchAllfromDB("");
            statusBarMessage("Import Finished");
            loadPacketsTable();
        }

    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Not a database");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Found no packets in this file. It may not be a Packet Sender export");
        msgBox.exec();
        return;
        statusBarMessage("Import Cancelled");
    }
}

void MainWindow::on_actionExport_Packets_triggered()
{

    static QString fileName = QDir::homePath() + QString("/packetsender_export.ini");

    fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                            QDir::toNativeSeparators(fileName), tr("INI db (*.ini)"));


    if (fileName.isEmpty()) {
        return;
    }

    QStringList testExt = fileName.split(".");
    QString ext = "";
    if (testExt.size() > 0) {
        ext = testExt.last();
    }
    if (ext != "ini") {
        fileName.append(".ini");
    }
    QDEBUGVAR(fileName);

    if (QFile::exists(fileName)) {
        QFile::remove(fileName);
    }

    QDEBUG() << QFile::copy(PACKETSFILE, fileName);
    statusBarMessage("Export: " + fileName);
}

void MainWindow::on_persistentTCPCheck_clicked(bool checked)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("persistentTCPCheck", ui->persistentTCPCheck->isChecked());

    packetNetwork.persistentConnectCheck = checked;
}


void MainWindow::on_actionIntense_Traffic_Generator_triggered()
{
    QDEBUG();
    quint16 port = ui->packetPortEdit->text().toUShort();
    UDPFlooding * f = new UDPFlooding(this, ui->packetIPEdit->text(), port, ui->packetASCIIEdit->text());
    f->show();

}



void MainWindow::on_actionSubnet_Calculator_triggered()
{
    QDEBUG();
    SubnetCalc * sCalc = new SubnetCalc(this);
    sCalc->show();

}

void MainWindow::on_resendEdit_editingFinished()
{
    float resendVal = Packet::oneDecimal(ui->resendEdit->text().toFloat());
    ui->resendEdit->setText(QString::number(resendVal));
}

void MainWindow::on_loadFileButton_clicked()
{
    static QString fileName;
    static bool showWarning = true;

    if (fileName.isEmpty()) {
        fileName = QDir::homePath();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Import File"),
                                            fileName,
                                            tr("*.*"));

    QDEBUGVAR(fileName);

    if (fileName.isEmpty()) {
        return;
    }

    QFile loadFile(fileName);

    if (!loadFile.exists()) {
        return;
    }

    QByteArray data;
    if (loadFile.open(QFile::ReadOnly)) {
        data = loadFile.readAll();
        loadFile.close();
        if (data.size() > (32767 / 3)) {
            data.resize(32767 / 3);

            if (showWarning) {
                showWarning = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle("Max size exceeded!");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("The hex field supports up to 10,922 bytes. The data has been truncated.");
                msgBox.exec();

            }

        }
        statusBarMessage("Loading " + QString::number(data.size()) + " bytes");
        ui->packetHexEdit->setText(Packet::byteArrayToHex(data));
        on_packetHexEdit_lostFocus();
        on_packetASCIIEdit_lostFocus();
        ui->packetASCIIEdit->setFocus();
        QDEBUGVAR(ui->packetHexEdit->text().size());
    }


}

void MainWindow::on_actionDonate_Thank_You_triggered()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://dannagle.com/paypal"));
}
