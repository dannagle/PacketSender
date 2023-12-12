/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
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
#include <QUrl>
#include <QUrlQuery>
#include <QPlainTextEdit>
#include <QMessageBox>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSortFilterProxyModel>


#include <QStringList>
#include <QSslCipher>


#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)

#else
#include <QRandomGenerator>
#endif


#include <QStandardPaths>

#include "brucethepoodle.h"
#include "irisandmarigold.h"
#include "settings.h"
#include "about.h"
#include "subnetcalc.h"
#include "udpflooding.h"
#include "cloudui.h"
#include "postdatagen.h"
#include "panelgenerator.h"

int MainWindow::isSessionOpen = false;
int hexToInt(QChar hex);
void parserMajorMinorBuild(QString sw, unsigned int &major, unsigned int &minor, unsigned int &build);
extern void themeTheButton(QPushButton * button);



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    QCheckBox* leaveSessionOpen;

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    if(settings.value("leaveSessionOpen").toString() == "false"){
        ui->leaveSessionOpen->setChecked(false);
    } else {
        ui->leaveSessionOpen->setChecked(true);
    }

    leaveSessionOpen = ui->leaveSessionOpen;
    connect(leaveSessionOpen, &QCheckBox::toggled, this, &MainWindow::on_leaveSessionOpen_StateChanged);

    cipherCb = ui->cipherCb;
    //add the combobox the correct cipher suites
    QList<QSslCipher> ciphers = QSslConfiguration::supportedCiphers();
    for (const QSslCipher &cipher : ciphers) {
        cipherCb->addItem(cipher.name());
    }

    if ( ui->udptcpComboBox->currentText().toLower() != "dtls"){
        ui->leaveSessionOpen->hide();
        cipherCb->hide();
        ui->CipherLable->hide();
    }

    connect(cipherCb, &QComboBox::editTextChanged, this, &MainWindow::on_cipherCb_currentIndexChanged);

    QIcon mIcon(":pslogo.png");


    lastSendPacket.clear();

    setWindowTitle("Packet Sender");
    setWindowIcon(mIcon);

    tableActive = false;
    darkMode = settings.value("darkModeCheck", true).toBool();

    PanelGenerator::darkMode = darkMode;

    maxLogSize = 10000;

    if (settings.value("rolling500entryCheck", false).toBool()) {
        maxLogSize = 100;
    }


    ui->generatePanelButton->hide();

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

    // default is TCP
    ui->udptcpComboBox->setCurrentIndex(ui->udptcpComboBox->findText("TCP"));

    //load last session
    if (settings.value("restoreSessionCheck", true).toBool()) {
        QDEBUG() << "Restoring last session";
        //ui->packetNameEdit->setText(settings.value("packetNameEditSession","").toString());
        ui->packetIPEdit->setText(settings.value("packetIPEditSession", "").toString());
        ui->packetHexEdit->setText(settings.value("packetHexEditSession", "").toString());
        ui->requestPathEdit->setText(settings.value("requestPathEditSession", "").toString());
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


    //update UI
    on_udptcpComboBox_currentIndexChanged(ui->udptcpComboBox->currentText());


    packetNetwork.sendResponse = settings.value("sendReponse", false).toBool();


    //http->get(QNetworkRequest(QUrl("http://packetsender.com/")));

    //Connect statusbar to packetNetwork
    //QDEBUG() << ": packetNetwork -> Statusbar connection attempt" <<
             connect(&packetNetwork, SIGNAL(toStatusBar(const QString &, int, bool)),
                     this, SLOT(statusBarMessage(const QString &, int, bool)));

    //Connect packetNetwork to trafficlog
    //QDEBUG() << ": packetSent -> toTrafficLog connection attempt" <<
             connect(&packetNetwork, SIGNAL(packetSent(Packet)),
                     this, SLOT(toTrafficLog(Packet)));

             connect(&packetNetwork, SIGNAL(packetReceived(Packet)), this, SLOT(toTrafficLog(Packet)));


    if( !QFile::exists(PACKETSFILE)) {
        // Packets file does not exist. Load starter set.
        QFile starterfile(":/starter_set.json");
        if (starterfile.open(QFile::ReadOnly)) {
            QList<Packet> packets = Packet::ImportJSON(starterfile.readAll());
            packetsImported(packets);
        }

    } else {
        packetsSaved = Packet::fetchAllfromDB("");
    }


    QFile starterPanelOut(PANELSFILE);

    if( !starterPanelOut.exists()) {
        // PS Panels file does not exist. Load starter panel.
        QFile starterPanel(":/ps_panels.json");
        if (starterPanel.open(QFile::ReadOnly)) {
            if(starterPanelOut.open(QFile::WriteOnly)) {
                starterPanelOut.write(starterPanel.readAll());
                starterPanelOut.close();
            }
            starterPanel.close();
        }

    } else {
        packetsSaved = Packet::fetchAllfromDB("");
    }

    QDEBUGVAR(packetsSaved.size());

    packetTableHeaders  = Settings::defaultTrafficTableHeader();
    packetTableHeaders = settings.value("packetTableHeaders", packetTableHeaders).toStringList();
    QStringList packetTableHeadersTranslated;
    foreach (QString pktHeader, packetTableHeaders) {
        packetTableHeadersTranslated.append(Settings::logHeaderTranslate(pktHeader));
    }
    packetsLogged.setTableHeaders(packetTableHeadersTranslated);


    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(&packetsLogged);
    ui->trafficLogTable->setModel(proxyModel);


    ui->trafficLogTable->verticalHeader()->show();
    ui->trafficLogTable->horizontalHeader()->show();

    ui->trafficLogTable->setSortingEnabled(true);


    loadPacketsTable();

//   statusBar()->insertPermanentWidget(0, generatePSLink());
//   statusBar()->insertPermanentWidget(1, generateDNLink());

    // handle double-clicking the ASCII window
    //asciiPreviewFilter = new PreviewFilter{ui->packetASCIIEdit, true};
    //hexPreviewFilter = new PreviewFilter{ui->packetHexEdit, false};

    asciiPreviewFilter = new PreviewFilter{ui->packetASCIIEdit, ui->packetASCIIEdit, ui->packetHexEdit};
    hexPreviewFilter = new PreviewFilter{ui->packetHexEdit, ui->packetASCIIEdit, ui->packetHexEdit};

    if (!connect(asciiPreviewFilter, &PreviewFilter::asciiUpdated, this, &MainWindow::on_packetASCIIEdit_lostFocus)) {
        QDEBUG() << "asciiPreviewFilter connection false";
    }

    if (!connect(hexPreviewFilter, &PreviewFilter::hexUpdated, this, &MainWindow::on_packetHexEdit_lostFocus)) {
        QDEBUG() << "hexPreviewFilter connection false";
    }

    stopResendingButton = new QPushButton(tr("Resending"));
    stopResendingButton->setStyleSheet(PersistentConnection::RESEND_BUTTON_STYLE);
    themeTheButton(stopResendingButton);
    stopResendingButton->setIcon(QIcon(PSLOGO));

    connect(stopResendingButton, SIGNAL(clicked()),
            this, SLOT(cancelResends()));


    statusBar()->insertPermanentWidget(1, stopResendingButton);

    stopResendingButton->hide();

    dtlsServerStatus = new QPushButton("DTLS:" + packetNetwork.getDTLSPortString());
    themeTheButton(dtlsServerStatus);
    dtlsServerStatus->setIcon(QIcon(UDPRXICON));

    connect(dtlsServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleDTLSServer()));


    statusBar()->insertPermanentWidget(2, dtlsServerStatus);

    udpServerStatus = new QPushButton("UDP:" + packetNetwork.getUDPPortString());
    themeTheButton(udpServerStatus);
    udpServerStatus->setIcon(QIcon(UDPRXICON));

    connect(udpServerStatus, SIGNAL(clicked()),
            this, SLOT(toggleUDPServer()));


    statusBar()->insertPermanentWidget(3, udpServerStatus);


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


    statusBar()->insertPermanentWidget(4, tcpServerStatus);


    statusBar()->insertPermanentWidget(5, sslServerStatus);



    //ipmode toggle
    IPmodeButton = new QPushButton("IPv4 Mode");
    themeTheButton(IPmodeButton);
    statusBar()->insertPermanentWidget(6, IPmodeButton);

    setIPMode();



    connect(IPmodeButton, SIGNAL(clicked()),
            this, SLOT(toggleIPv4_IPv6()));

    DTLSServerStatus();
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



    //Bruce was my wonderful poodle rescue. 2008-2021
    //Dog easter egg.  CTRL D, O, G.
    //             or  CMD D, O, G.
    QShortcut *shortcutDog = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D, Qt::CTRL + Qt::Key_O, Qt::CTRL + Qt::Key_G), this);
    QDEBUG() << ": dog easter egg Connection attempt " << connect(shortcutDog, SIGNAL(activated()), this, SLOT(poodlepic()));


    //Iris and Marigold are my rescue puppies.
    //Pup easter egg.  CTRL P, U, P.
    //             or  CMD P, U, P.
    QShortcut *shortcutPup = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_P, Qt::CTRL + Qt::Key_U, Qt::CTRL + Qt::Key_P), this);
    QDEBUG() << ": puppy easter egg Connection attempt " << connect(shortcutPup, SIGNAL(activated()), this, SLOT(puppypic()));


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



    // QTimer::singleShot(100.0, [this](){ this->timeout();});

    if(settings.value("autolaunchStarterPanelButton", false).toBool()) {
        statusBarMessage("Auto-launching starter panel.", 3000, true);

    }

    // Delayed to give message time to display, and make sure this is on top.
    QTimer::singleShot(1500, this, [this] () {
        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        if(settings.value("autolaunchStarterPanelButton", false).toBool()) {
            QDEBUG() << "Auto-launching starter panel";
            this->on_actionPanel_Generator_triggered();
        }

    } );








    QDEBUG() << "Settings file loaded" << SETTINGSFILE;
    QDEBUG() << "Packets file loaded" << PACKETSFILE;

    // Generate starter_set.json
    /*
    QByteArray j = Packet::ExportJSON(packetsSaved);
#ifdef _WIN32
    QFile starter("../src/starter_set.json");
#else
    QFile starter("starter_set.json");
#endif
    starter.open(QFile::WriteOnly);
    starter.write(j);
    starter.close();
    */




}





void MainWindow::toggleDTLSServer()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("dtlsServerEnable", !settings.value("dtlsServerEnable", true).toBool());
    applyNetworkSettings();
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

    //snaps do not need to check for updates.
#ifdef ISSNAP
    return;
#endif

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
                msgBox.setWindowTitle(tr("Updates."));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("There is a new Packet Sender available.\n\nDownload?"));
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

    if (!settings.value("checkforUpdatesAsked", false).toBool()) {
        settings.setValue("checkforUpdatesAsked", true);
        settings.setValue("SW_VERSION", SW_VERSION); // first run. Save the current version.
        QMessageBox msgBox;
        msgBox.setWindowIcon(QIcon(":pslogo.png"));
        msgBox.setWindowTitle(tr("Updates."));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Let Packet Sender check for updates weekly?"));
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::Yes) {
            QDEBUG() << "Will check for updates";
            settings.setValue("checkforUpdates", true);
        } else {
            QDEBUG() << "Will NOT check for updates";
            settings.setValue("checkforUpdates", false);
        }

        settings.sync();
    } else {

        //This is not the first run.
        QString previousVersion = settings.value("SW_VERSION", "unknown").toString();
        QString swCheck = SW_VERSION;

        // remove v prefix (not used in all installations)
        previousVersion.replace("v", "");
        swCheck.replace("v", ""); // remove v prefix (not used in all installations)

        // We only pop the update prompt if it is a newer version we haven't seen.
        QStringList previousList = previousVersion.split(".");
        QStringList swList = swCheck.split(".");
        int swListCount = 1;
        int prevListCount = 0;
        if((swList.size() == 3) && (previousList.size() == 3)) {
            swListCount = swList[0].toUInt() * 10000 + swList[1].toUInt() * 100 + swList[2].toUInt();
            prevListCount = previousList[0].toUInt() * 10000 + previousList[1].toUInt() * 100 + previousList[2].toUInt();
            if(swListCount != prevListCount) {
                if(swListCount > prevListCount) {
                    QDEBUG() << "New version unseen before. We pop the prompt!";
                } else {
                    QDEBUG() << "This is an older version. Do not prompt.";
                }
            }
        }


        if((swListCount > prevListCount) && (previousVersion != swCheck)) {
            QDEBUG() << "New version detected:" << previousVersion << "!=" << swCheck;
            settings.setValue("SW_VERSION", swCheck);
            QMessageBox msgBox;
            msgBox.setWindowIcon(QIcon(":pslogo.png"));
            msgBox.setWindowTitle(tr("Packet Sender Updated!"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Updated to ") + swCheck + tr("!\n\nWould you like to read the release notes?"));
            int yesno = msgBox.exec();
            if (yesno == QMessageBox::Yes) {
                QDesktopServices::openUrl(QUrl("https://github.com/dannagle/PacketSender/releases/latest"));
            }
        }
    }



    if (settings.value("checkforUpdates", true).toBool()) {
        QString updateLastCheckedString = settings.value("updateLastChecked").toString();
        QDateTime updateLastChecked = QDateTime::fromString(updateLastCheckedString, FULLDATETIMEFORMAT);
        if(QFile::exists(QDir::homePath() + "/ALWAYSUPDATE")) {
            QDEBUG() << "Always check updates enabled.";
            updateLastChecked  = QDateTime::fromString("2019-07-04 11:46:52", FULLDATETIMEFORMAT);
        }
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
    ui->trafficLogClearButton->setText(tr("Clear Log ") +"("+QString::number(trafficlogsize)+")");


    if(!initialpackets) {

        ui->trafficLogTable->resizeColumnsToContents();
        ui->trafficLogTable->resizeRowsToContents();
        ui->trafficLogTable->horizontalHeader()->setStretchLastSection(true);

        initialpackets = true;

    }


}

void MainWindow::DTLSServerStatus()
{

    if (packetNetwork.DTLSListening()) {
        QString ports = packetNetwork.getDTLSPortString();
        int portcount = packetNetwork.getDTLSPortsBound().size();
        dtlsServerStatus->setToolTip(ports);
        if(portcount > 3) {
            dtlsServerStatus->setText("DTLS: " + QString::number(portcount) + tr(" Ports"));
        } else {
            dtlsServerStatus->setText("DTLS:" + ports);
        }

    } else {
        dtlsServerStatus->setText(tr("DTLS Server Disabled"));

    }

    //updatewidget


}

void MainWindow::UDPServerStatus()
{

    if (packetNetwork.UDPListening()) {
        QString ports = packetNetwork.getUDPPortString();
        int portcount = packetNetwork.getUDPPortsBound().size();
        udpServerStatus->setToolTip(ports);
        if(portcount > 3) {
            udpServerStatus->setText("UDP: " + QString::number(portcount) + tr(" Ports"));
        } else {
            udpServerStatus->setText("UDP:" + ports);
        }

    } else {
        udpServerStatus->setText(tr("UDP Server Disabled"));

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
            sslServerStatus->setText("SSL: " + QString::number(portcount) + tr(" Ports"));
        } else {
            sslServerStatus->setText("SSL:" + ports);
        }

    } else {
        sslServerStatus->setText(tr("SSL Server Disabled"));

    }


}


void MainWindow::TCPServerStatus()
{
    if (packetNetwork.TCPListening()) {
        QString ports = packetNetwork.getTCPPortString();
        int portcount = packetNetwork.getTCPPortsBound().size();
        tcpServerStatus->setToolTip(ports);
        if(portcount > 3) {
            tcpServerStatus->setText("TCP: " + QString::number(portcount) + tr(" Ports"));
        } else {
            tcpServerStatus->setText("TCP:" + ports);
        }


    } else {
        tcpServerStatus->setText(tr("TCP Server Disabled"));

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



    QStringList originalpacketSavedTableHeaders  = Settings::defaultPacketTableHeader();
    packetSavedTableHeaders = settings.value("packetSavedTableHeaders", originalpacketSavedTableHeaders ).toStringList();
    QString saveTest;
    foreach(saveTest, packetSavedTableHeaders) {
        if(!originalpacketSavedTableHeaders.contains(saveTest)) {
            packetSavedTableHeaders = originalpacketSavedTableHeaders;
            break;
        }
    }
    if(packetSavedTableHeaders.size() != originalpacketSavedTableHeaders.size()) {
        packetSavedTableHeaders = originalpacketSavedTableHeaders;
    }

    ui->packetsTable->setColumnCount(packetSavedTableHeaders.size());

    ui->packetsTable->verticalHeader()->show();
    ui->packetsTable->horizontalHeader()->show();

    QStringList packetTableHeadersTranslated;
    foreach (QString pktHeader, packetSavedTableHeaders) {
        packetTableHeadersTranslated.append(Settings::logHeaderTranslate(pktHeader));
    }

    ui->packetsTable->setHorizontalHeaderLabels(packetTableHeadersTranslated);
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

    pReply->deleteLater();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_packetHexEdit_lostFocus()
{

    QString quicktestHex =  ui->packetHexEdit->text();

    ui->packetASCIIEdit->setText(Packet::hexToASCII(quicktestHex));
    ui->packetASCIIEdit->setToolTip("");
    ui->packetHexEdit->setText(quicktestHex);


    QTimer::singleShot(100.0, [this](){
        ui->packetHexEdit->deselect();
        ui->packetHexEdit->setCursorPosition(ui->packetHexEdit->text().size());
    });


}



void MainWindow::on_requestPathEdit_lostFocus()
{
    QDEBUG();

    auto isHttp = ui->udptcpComboBox->currentText().toLower().contains("http");

    QString quicktestURL =  ui->requestPathEdit->text().trimmed().toLower();
    auto checkHTTP = quicktestURL.startsWith("http://") || quicktestURL.startsWith("https://");

    if(isHttp && checkHTTP) {

        ui->packetPortEdit->setText(QString::number(Packet::getPortFromURL(quicktestURL)));
        ui->packetIPEdit->setText(Packet::getHostFromURL(quicktestURL));
        ui->requestPathEdit->setText(Packet::getRequestFromURL(quicktestURL));
        int index = ui->udptcpComboBox->findText(Packet::getMethodFromURL(quicktestURL));
        if(index >= 0) {
            ui->udptcpComboBox->setCurrentIndex(index);
        }
    }
}

void MainWindow::on_packetASCIIEdit_lostFocus()
{
    QDEBUG();

    QString quicktestASCII =  ui->packetASCIIEdit->text();
    ui->packetHexEdit->setText(Packet::ASCIITohex(quicktestASCII));


    QString quicktestASCII2 =  ui->packetHexEdit->text();

    ui->packetASCIIEdit->setText(Packet::hexToASCII(quicktestASCII2));
    ui->packetASCIIEdit->setToolTip("");

    QTimer::singleShot(100.0, [this](){
        ui->packetASCIIEdit->deselect();
        ui->packetASCIIEdit->setCursorPosition(ui->packetASCIIEdit->text().size());
    });



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
                msgBox.setWindowTitle(tr("Multicast detected."));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Do you wish to join the multicast group?"));
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
                    msgBox.setWindowTitle(tr("Broadcast with responses!"));
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::No);
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText(tr("You are sending a broadcast packet with responses enabled.\n\nThis could cause traffic flooding. Continue?"));
                    int yesno = msgBox.exec();
                    if (yesno == QMessageBox::No) {
                        return;
                    }

                }
            }

            if (toSend.repeat > 0) {
                toSend.timestamp = QDateTime::currentDateTime();

                stopResendingButton->setStyleSheet("QPushButton { color: green; background-color: #505F69; } QPushButton::hover { color: #BC810C; background-color: #505F69; } ");
                packetsRepeat.append(toSend);
                stopResendingButton->setText(tr("Resending") +" (" + QString::number(packetsRepeat.size()) + ")");
                stopResending = 0;
            }

            lastSendPacket = toSend;
            QByteArray sendData = toSend.getByteArray();
            statusBarMessage(tr("Send")+": " + packetName + "  (" + QString::number(sendData.size()) + " "+tr("bytes")+")");
            emit sendPacket(toSend);
        }
    }

}

void MainWindow::on_savePacketButton_clicked()
{


    if(ui->packetASCIIEdit->hasFocus()) {
        QDEBUG() << "Forcing ASCII edit trigger";
        on_packetASCIIEdit_editingFinished();
    }

    if(ui->packetHexEdit->hasFocus()) {
        QDEBUG() << "Forcing HEX edit trigger";
        on_packetHexEdit_editingFinished();
    }

    Packet testPacket;
    testPacket.init();
    testPacket.name = ui->packetNameEdit->text().trimmed();
    if (testPacket.name.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setText(tr("Name cannot be blank."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Name is empty."));
        msgBox.exec();
        ui->packetNameEdit->setFocus();
        return;

    }
    testPacket.toIP = ui->packetIPEdit->text().trimmed();
    testPacket.hexString =  ui->packetHexEdit->text().simplified();
    testPacket.tcpOrUdp = ui->udptcpComboBox->currentText();
    testPacket.requestPath = ui->requestPathEdit->text().trimmed();
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
    Q_UNUSED(sessionPacket)

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    //settings.setValue("packetNameEditSession", ui->packetNameEdit->text());
    settings.setValue("packetIPEditSession", ui->packetIPEdit->text());
    settings.setValue("packetHexEditSession", ui->packetHexEdit->text());
    settings.setValue("requestPathEditSession", ui->requestPathEdit->text());
    settings.setValue("udptcpComboBoxSession", ui->udptcpComboBox->currentText());
    settings.setValue("packetPortEditSession", ui->packetPortEdit->text());
    settings.setValue("resendEditSession", ui->resendEdit->text());

}


void MainWindow::on_testPacketButton_clicked()
{
    Packet testPacket;
    static QStringList noMCastList;
    testPacket.init();

    if ((ui->udptcpComboBox->currentText() == "SSL") || ui->udptcpComboBox->currentText().startsWith("HTTPS")) {

        if (!QSslSocket::supportsSsl()) {

            QMessageBox msgBox;
            msgBox.setText(tr("This computer does not support SSL.\n\nExpected SSL:") + QSslSocket::sslLibraryVersionString());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle(tr("No SSL Library."));
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
    testPacket.requestPath =  ui->requestPathEdit->text();

    //Save Session!
    saveSession(testPacket);

    if (testPacket.toIP.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setText(tr("Address cannot be blank."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Address is empty."));
        msgBox.exec();
        ui->packetIPEdit->setFocus();
        return;

    }


    if (testPacket.port == 0) {

        QMessageBox msgBox;
        msgBox.setText(tr("Port cannot be blank/zero."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Port is zero."));
        msgBox.exec();
        ui->packetPortEdit->setFocus();
        return;

    }



    bool isIPv6 = testPacket.toIP.contains(":");

    if(isIPv6 && (!packetNetwork.IPv6Enabled())) {

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("IPv6?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Enable IPv6 support?"));
        // Packet Sender now auto-switches
        /*
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::Yes) {
            packetNetwork.setIPmode(6);
            packetNetwork.kill();
            packetNetwork.init();
        }
        */
    }


    if (PacketNetwork::isMulticast(testPacket.toIP) && (!noMCastList.contains(testPacket.toIP))) {

        //are we joined?



        if(!packetNetwork.canSendMulticast(testPacket.toIP)) {

            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Multicast detected."));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Join UDP multicast group?"));

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
            msgBox.setWindowTitle(tr("Broadcast with responses!"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("You are sending a broadcast packet with responses enabled.\n\nThis could cause traffic flooding. Continue?"));
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
            msgBox.setWindowTitle(tr("Resend TCP with persistent connections!"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("You are resending a TCP packet with persistent connections. The UI could spawn numerous windows! \n\nUncheck persistent connection? (Recommended)"));
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

        stopResendingButton->setStyleSheet("QPushButton { color: green; background-color: #505F69; } QPushButton::hover { color: #BC810C; background-color: #505F69; } ");
        packetsRepeat.append(testPacket);
        stopResendingButton->setText(tr("Resending") +" (" + QString::number(packetsRepeat.size()) + ")");
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
        msgBox.setText(tr("No packets selected."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(tr("Select a packet."));
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
            ui->packetIPEdit->setPlaceholderText(tr("Invalid Address / DNS failed"));
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
        ui->packetPortEdit->setPlaceholderText(tr("Invalid Port"));
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
    if (datatype == Settings::NAME_STR) {
        Packet::removeFromDB(updatePacket.name); //remove old before inserting new.
        removePacketfromMemory(updatePacket);
        updatePacket.name = newText;
        fullrefresh = 1;
    }
    if (datatype == Settings::TOADDRESS_STR) {
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
    if (datatype == Settings::TOPORT_STR) {
        int portNum = newText.toUInt();
        if (portNum > 0) {
            updatePacket.port = portNum;
        }
    }
    if (datatype == Settings::RESEND_STR) {
        float repeat = Packet::oneDecimal(newText.toFloat());
        updatePacket.repeat = repeat;
    }
    if (datatype == Settings::METHOD_STR) {
        if ((newText.trimmed().toUpper() == "TCP") || (newText.trimmed().toUpper() == "UDP") || (newText.trimmed().toUpper() == "SSL")) {
            updatePacket.tcpOrUdp = newText.trimmed().toUpper();
        }
        auto isHTTP = newText.trimmed().toUpper().contains("HTTP") || newText.trimmed().toUpper().contains("GET") || newText.trimmed().toUpper().contains("POST");

        if(isHTTP) {
            auto isHTTPS = (newText.trimmed().toUpper().contains("HTTPS"));
            auto isPOST = newText.trimmed().toUpper().contains("POST");
            updatePacket.tcpOrUdp = "HTTP";
            if(isHTTPS) {
                updatePacket.tcpOrUdp.append("S");
            }
            if(isPOST) {
                updatePacket.tcpOrUdp.append(" Post");
            } else {
                updatePacket.tcpOrUdp.append(" Get");
            }
        }


    }
    if (datatype == Settings::ASCII_STR) {
        QString hex = Packet::ASCIITohex(newText);
        updatePacket.hexString = hex;
    }

    if (datatype == Settings::REQUEST_STR) {
        updatePacket.requestPath = newText;
    }

    if (datatype == Settings::HEX_STR) {
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

void MainWindow::puppypic()
{
    QDEBUG();

    IrisAndMarigold *pups = new IrisAndMarigold(this);
    pups->show();
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



    ui->packetsTable->setCellWidget(rowCounter, packetSavedTableHeaders.indexOf("Send"), sendButton);

    tItem = new QTableWidgetItem(QString::number(tempPacket.repeat));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::RESEND_STR);
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::RESEND_STR), tItem);
    //QDEBUGVAR(tempPacket.name);

    tItem = new QTableWidgetItem(tempPacket.name);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::NAME_STR);
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::NAME_STR), tItem);
    //QDEBUGVAR(tempPacket.name);

    tItem = new QTableWidgetItem(tempPacket.toIP);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::TOADDRESS_STR);

    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::TOADDRESS_STR), tItem);
    //QDEBUGVAR(tempPacket.toIP);

    tItem = new QTableWidgetItem(QString::number(tempPacket.port));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::TOPORT_STR);
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::TOPORT_STR), tItem);
    // QDEBUGVAR(tempPacket.port);

    tItem = new QTableWidgetItem(tempPacket.tcpOrUdp);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::METHOD_STR);
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::METHOD_STR), tItem);
    // QDEBUGVAR(tempPacket.tcpOrUdp);

    tItem = new QTableWidgetItem(tempPacket.hexToASCII(tempPacket.hexString));
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::ASCII_STR);

    if(tempPacket.isHTTP()) {
        tItem->setText(tempPacket.requestPath);
        tItem->setData(Packet::DATATYPE, Settings::REQUEST_STR);
    }


    QSize tSize = tItem->sizeHint();
    tSize.setWidth(200);
    tItem->setSizeHint(tSize);

    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::ASCII_STR), tItem);
    //QDEBUGVAR(tempPacket.hexString);

    tItem = new QTableWidgetItem(tempPacket.hexString);
    Packet::populateTableWidgetItem(tItem, tempPacket);
    tItem->setData(Packet::DATATYPE, Settings::HEX_STR);
    ui->packetsTable->setItem(rowCounter, packetSavedTableHeaders.indexOf(Settings::HEX_STR), tItem);
    if(tempPacket.isHTTP()) {
        if(tempPacket.isPOST()) {
            tItem->setText(tempPacket.asciiString());
        } else {
            tItem->setText("");
        }
        tItem->setData(Packet::DATATYPE, Settings::ASCII_STR);

    }


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

    ui->generatePanelButton->hide();

    foreach (checkItem, totalSelected) {
        clickedPacket = Packet::fetchTableWidgetItemData(checkItem);
        if (buttonsList.contains(clickedPacket.name)) {
            continue;
        } else {
            packetList.append(clickedPacket);
            buttonsList.append(clickedPacket.name);
        }
    }

    if (packetList.size() > 1) {
       ui->generatePanelButton->show();
    }

    ui->testPacketButton->setText(tr("Send"));
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
            ui->requestPathEdit->setText(clickedPacket.requestPath);
            ui->packetIPEdit->setText(clickedPacket.toIP);
            ui->packetPortEdit->setText(QString::number(clickedPacket.port));
            ui->resendEdit->setText(QString::number(clickedPacket.repeat));
            QString oldText = ui->udptcpComboBox->currentText();
            ui->udptcpComboBox->setCurrentIndex(ui->udptcpComboBox->findText(clickedPacket.tcpOrUdp));
            ui->packetASCIIEdit->setText(Packet::hexToASCII(clickedPacket.hexString));
            ui->packetASCIIEdit->setToolTip("");

            if(oldText != clickedPacket.tcpOrUdp) {
                on_udptcpComboBox_currentIndexChanged(clickedPacket.tcpOrUdp);
            }

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
        packetsLogged.removeLast();
    }


        //ui->mainTabWidget->setTabText(1,"Traffic Log (" + QString::number(packetsLogged.size()) +")");

    //got nothing else to do. check datagrams.
    packetNetwork.readPendingDatagrams();


}

void MainWindow::on_trafficLogClearButton_clicked()
{
    packetsLogged.clear();
    Packet tempPacket;
    ui->trafficLogClearButton->setText(tr("Clear Log (0)"));
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
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        Packet savePacket = packetsLogged.getPacket(sourceIndex);
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

    packetsLogged.useEllipsis = settings.value("ellipsisCheck", true).toBool();

    packetNetwork.kill();
    packetNetwork.init();
    packetNetwork.responseData = settings.value("responseHex", "").toString().trimmed();
    packetNetwork.sendResponse = settings.value("sendReponse", false).toBool();
    ui->persistentTCPCheck->setChecked(settings.value("persistentTCPCheck", false).toBool());
    on_persistentTCPCheck_clicked(ui->persistentTCPCheck->isChecked());

    DTLSServerStatus();
    UDPServerStatus();
    TCPServerStatus();
    SSLServerStatus();

}

void MainWindow::cancelResends()
{
    stopResendingButton->setStyleSheet(PersistentConnection::RESEND_BUTTON_STYLE);
    stopResendingButton->setText(tr("Resending"));
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

void MainWindow::on_requestPathEdit_editingFinished()
{
    on_requestPathEdit_lostFocus();
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
        msgBox.setText(tr("No packets selected."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(tr("Clipboard unchanged."));
        msgBox.exec();
        return;
    }
    foreach (index, indexes) {
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        Packet savePacket = packetsLogged.getPacket(sourceIndex);
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
        statusBarMessage("Copied raw packet data.");
    } else {
        clipboard->setText(clipString);
        statusBarMessage("Copied translated packet data.");
    }

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
    QDesktopServices::openUrl(QUrl("http://forums.naglecode.com/"));


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

    out << tr("TIME") << delim << tr("From IP") << delim << tr("From Port") << delim << tr("To IP")
        << delim << tr("To Port") << delim << tr("Method") << delim << tr("Error") << delim << "ASCII" << delim << "Hex\n";


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
    themeTheButton(IPmodeButton);

    QString islight = "";

    if(darkMode) {
        islight = "light";
    }

    if(isIPv6) {
        IPmodeButton->setStyleSheet("QPushButton { color: " + islight +"blue}");
    } else {
        IPmodeButton->setStyleSheet("QPushButton { color:  " + islight +"green}");
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

    QString language = Settings::language().toLower();

    if(language.contains("spanish")) {
        QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation-es"));
        return;
    }


    if(language.contains("german")) {
        QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation-de"));
        return;
    }


    if(language.contains("french")) {
        QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation-fr"));
        return;
    }


    QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation"));
}

void MainWindow::on_leaveSessionOpen_StateChanged(){
    //ui.checkBox->setChecked(checkBoxState);

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString leaveSessionOpen = settings.value("leaveSessionOpen", "false").toString();
    if(leaveSessionOpen == "false"){
        settings.setValue("leaveSessionOpen", "true");
    }
    else{
        settings.setValue("leaveSessionOpen", "false");
    }
}

void MainWindow::on_sendSimpleAck_StateChanged(){
    //ui.checkBox->setChecked(checkBoxState);

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString sendSimpleAck = settings.value("sendSimpleAck", "false").toString();
    if(sendSimpleAck == "false"){
        settings.setValue("sendSimpleAck", "true");
    }
    else{
        settings.setValue("sendSimpleAck", "false");
    }
}

void MainWindow::on_actionSettings_triggered()
{
    Settings settings(this);
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
        msgBox.setWindowTitle(tr("Found ") + QString::number(importList.size()) + tr(" packets!"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Import ") + QString::number(importList.size()) + tr(" packets?\n\nPacket Sender will overwrite packets with the same name."));
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::No) {
            statusBarMessage(tr("Import Cancelled"));
            return;
        } else {
            foreach (importPacket, importList) {
                importPacket.saveToDB();

            }
            packetsSaved = Packet::fetchAllfromDB("");
            statusBarMessage(tr("Import Finished"));
            loadPacketsTable();
        }

    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Not a database"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Found no packets in this file. It may not be a Packet Sender export"));
        msgBox.exec();
        return;
        statusBarMessage(tr("Import Cancelled"));
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
        statusBarMessage(tr("Export: ") + fileName);
    } else {

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Could not save"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Could not open ") + fileName + (" for saving."));
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
        msgBox.setWindowTitle(tr("Found ") + QString::number(importList.size()) + tr(" packets!"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Import ") + QString::number(importList.size()) + tr(" packets?\n\nPacket Sender will overwrite packets with the same name."));
        int yesno = msgBox.exec();
        if (yesno == QMessageBox::No) {
            statusBarMessage(tr("Import Cancelled"));
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
        msgBox.setWindowTitle(tr("Not a database"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Found no packets in this file. It may not be a Packet Sender export"));
        msgBox.exec();
        return;
        statusBarMessage(tr("Import Cancelled"));
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
    statusBarMessage(tr("Export: ") + fileName);
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
                msgBox.setWindowTitle(tr("Max size exceeded!"));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("The HEX field supports up to 10,922 bytes. The data has been truncated."));
                msgBox.exec();

            }

        }
        statusBarMessage(tr("Loading ") + QString::number(data.size()) + tr(" bytes"));
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

void MainWindow::on_udptcpComboBox_currentIndexChanged(const QString &arg1)
{

    QString selectedText = ui->udptcpComboBox->currentText().toLower();
    auto isHttp = selectedText.contains("http");
    auto isPost = selectedText.contains("post") && isHttp;

    /////////////////////////////////dtls add line edit for adding path for cert

    if ( ui->udptcpComboBox->currentText().toLower() == "dtls") {
        ui->leaveSessionOpen->show();
        cipherCb->show();
        ui->CipherLable->show();
    } else {
        ui->leaveSessionOpen->hide();
        cipherCb->hide();
        ui->CipherLable->hide();

    }


    if(isHttp) {
        ui->asciiLabel->setText("Data");
    } else {
        ui->asciiLabel->setText("ASCII");
    }


    for (int i = 0; i < ui->hexHorizLayout->count(); ++i) {
        QWidget *w = ui->hexHorizLayout->itemAt(i)->widget();
        if(w != nullptr) {
            w->setVisible(!isHttp);
        }
    }

    for (int i = 0; i < ui->requestLayout->count(); ++i) {
        QWidget *w = ui->requestLayout->itemAt(i)->widget();
        if(w != nullptr) {
            w->setVisible(isHttp);
        }
    }

    for (int i = 0; i < ui->asciiLayout->count(); ++i) {
        QWidget *w = ui->asciiLayout->itemAt(i)->widget();
        if(w != nullptr) {
            w->setVisible((!isHttp) || isPost);
        }
    }

    ui->genPostDataButton->setVisible(isPost);
}

void MainWindow::on_cipherCb_currentIndexChanged(){
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("cipher", cipherCb->currentText());
    //create new session even if the leave open session checkbox is pushed create new session, because the cipher has been changed
    isSessionOpen = false;

}


void MainWindow::on_genPostDataButton_clicked()
{
    PostDataGen * phttp = new PostDataGen(this, ui->packetASCIIEdit->text());


    bool ready = connect(phttp, &PostDataGen::postGenerated, this, [=](QString val) {
        // use action as you wish
        QDEBUGVAR(val);
        ui->packetASCIIEdit->setText(val);

        on_packetASCIIEdit_editingFinished();
    });

    if (!ready) {
        QDEBUG() << "postGenerated connection false";
    }


    phttp->show();


}

void MainWindow::on_generatePanelButton_clicked()
{

    QList<Packet> packetList;
    QModelIndexList indexes = ui->packetsTable->selectionModel()->selectedIndexes();
    QModelIndex index;
    QString selected, name;

    QStringList nameList;
    nameList.clear();

    foreach (index, indexes) {
        selected = index.data(Packet::PACKET_NAME).toString();
        if(selected.isEmpty()) {
            continue;
        }
        if (!nameList.contains(selected)) {
            nameList.append(selected);
        }
    }

    foreach (name, nameList) {
        Packet pkt = Packet::fetchFromDB(name);
        packetList.append(pkt);
    }

    PanelGenerator * gpanel = new PanelGenerator(this);

    QDEBUG() << " packet send connect attempt:" << connect(gpanel, SIGNAL(sendPacket(Packet)),
             &packetNetwork, SLOT(packetToSend(Packet)));


    gpanel->init(packetList);
    gpanel->show();
}


void MainWindow::on_actionPanel_Generator_triggered()
{
    PanelGenerator * gpanel = new PanelGenerator(this);

    QDEBUG() << " packet send connect attempt:" << connect(gpanel, SIGNAL(sendPacket(Packet)),
             &packetNetwork, SLOT(packetToSend(Packet)));

    gpanel->initAutoLaunchOrEditMode();
    gpanel->show();

}


void MainWindow::on_actionNew_Panel_triggered()
{
    PanelGenerator * gpanel = new PanelGenerator(this);

    QDEBUG() << " packet send connect attempt:" << connect(gpanel, SIGNAL(sendPacket(Packet)),
             &packetNetwork, SLOT(packetToSend(Packet)));
    gpanel->show();
}

void MainWindow::on_testPacketButton_pressed()
{
    if(ui->packetASCIIEdit->hasFocus()) {
        QDEBUG() << "Forcing ASCII edit trigger";
        on_packetASCIIEdit_editingFinished();
    }

    if(ui->packetHexEdit->hasFocus()) {
        QDEBUG() << "Forcing HEX edit trigger";
        on_packetHexEdit_editingFinished();
    }

}

bool PreviewFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {

        QDialog previewDlg;
        previewDlg.setWindowFlags(previewDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        previewDlg.setWindowTitle(tr("Multi-line editor"));
        QVBoxLayout* layout = new QVBoxLayout{&previewDlg};
        QPlainTextEdit* editor = new QPlainTextEdit{&previewDlg};
        QLineEdit* _lineEdit = dynamic_cast<QLineEdit*>(parent());
        _lineEdit->deselect();

        QPushButton* updateButton = new QPushButton(tr("Update"));
        QPushButton* closeButton = new QPushButton(tr("Close"));
        QObject::connect(closeButton, &QPushButton::clicked,
            [&previewDlg]
            {
                previewDlg.close();
            });

        QObject::connect(updateButton, &QPushButton::clicked,
            [&previewDlg, &editor, &_lineEdit, this]
            {
                QString contents = editor->toPlainText();
                if(_lineEdit == this->asciiEdit) {
                    this->asciiEdit->setText(contents);
                    emit asciiUpdated();
                }
                if(_lineEdit == this->hexEdit) {
                    this->hexEdit->setText(contents);
                    emit hexUpdated();
                }
                previewDlg.close();
            });

        layout->addWidget(editor);
        layout->addWidget(updateButton);
        layout->addWidget(closeButton);

        previewDlg.setLayout(layout);
        previewDlg.setModal(true);

        // we have to make a round trip, ascii to hex and then back to ascii with whitespace
        auto asciidata = _lineEdit->text();
        if(_lineEdit == this->hexEdit)
        {
            editor->setPlainText(asciidata.simplified() + " ");
        }
        if(_lineEdit == this->asciiEdit)
        {
            //do round trip conversion
            asciidata = Packet::ASCIITohex(asciidata);
            auto br = Packet::HEXtoByteArray(asciidata);
            editor->setPlainText(QString(br));
        }
        editor->moveCursor(QTextCursor::End);

        previewDlg.exec();
    }

    return QObject::eventFilter(watched, event);
}

void MainWindow::on_udptcpComboBox_currentIndexChanged(int index)
{
    on_udptcpComboBox_currentIndexChanged("");
}

