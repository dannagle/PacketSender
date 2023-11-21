#include "settings.h"


#include <QStringList>
#include <QDebug>
#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QPair>
#include <QFile>
#include <QHostAddress>
#include <QStandardPaths>
#include <QObject>


#ifndef CONSOLE_BUILD
#include "ui_settings.h"
#include "languagechooser.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#endif

const QString Settings::SEND_STR = "Send";
const QString Settings::NAME_STR = "Name";
const QString Settings::RESEND_STR = "Resend";
const QString Settings::TOADDRESS_STR = "To Address";
const QString Settings::TOPORT_STR = "To Port";
const QString Settings::METHOD_STR = "Method";
const QString Settings::ASCII_STR = "ASCII";
const QString Settings::HEX_STR = "Hex";
const QString Settings::REQUEST_STR = "Request Path";

const QString Settings::TIME_STR = "Time";
const QString Settings::FROMIP_STR = "From IP";
const QString Settings::FROMPORT_STR = "From Port";
const QString Settings::ERROR_STR = "Error";



QString Settings::logHeaderTranslate(QString txt)
{
    if(txt == "Send") {
        return QObject::tr("Send");
    }
    if(txt == "Name") {
        return QObject::tr("Name");
    }
    if(txt == "Resend") {
        return QObject::tr("Resend");
    }
    if(txt == "To Address") {
        return QObject::tr("To Address");
    }
    if(txt == "To Port") {
        return QObject::tr("To Port");
    }
    if(txt == "Method") {
        return QObject::tr("Method");
    }
    if(txt == "Request Path") {
        return QObject::tr("Request Path");
    }
    if(txt == "Time") {
        return QObject::tr("Time");
    }
    if(txt == "From IP") {
        return QObject::tr("From IP");
    }
    if(txt == "From Port") {
        return QObject::tr("From Port");
    }
    if(txt == "Error") {
        return QObject::tr("Error");
    }

    return txt;

}


const QString Settings::ALLHTTPSHOSTS = "HTTPHeaderHosts";
const QString Settings::HTTPHEADERINDEX = "HTTPHeader:";

#ifndef CONSOLE_BUILD

Settings::Settings(QWidget *parent, MainWindow* mw) :
    QDialog(parent),
    rmw(mw),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //not working yet...
    ui->multiSendDelayLabel->hide();
    ui->multiSendDelayEdit->hide();
    //connect(loadKeyButton, &QPushButton::clicked, this, &MainWindow::on_loadKeyButton_clicked);

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    if(settings.value("leaveSessionOpen").toString() == "false"){
        ui->leaveSessionOpen->setChecked(false);
    } else {
        ui->leaveSessionOpen->setChecked(true);
    }

    QIcon mIcon(":pslogo.png");
    setWindowTitle("Packet Sender "+tr("Settings"));
    setWindowIcon(mIcon);

    //this is no longer working thanks to faster traffic log
    ui->displayOrderListTraffic->hide();
    ui->displayGroupBoxTraffic->setTitle("");

    loadCredentialTable();
    on_genAuthCheck_clicked(false);

    //smart responses...
    ui->smartResponseEnableCheck->setChecked(settings.value("smartResponseEnableCheck", false).toBool());

#define RESPONSEIF(NUM) ui->responseIfEdit##NUM->setText(settings.value("responseIfEdit" #NUM,"").toString())

    RESPONSEIF(1);
    RESPONSEIF(2);
    RESPONSEIF(3);
    RESPONSEIF(4);
    RESPONSEIF(5);

#define RESPONSEREPLY(NUM) ui->responseReplyEdit##NUM->setText(settings.value("responseReplyEdit"#NUM,"").toString())

    RESPONSEREPLY(1);
    RESPONSEREPLY(2);
    RESPONSEREPLY(3);
    RESPONSEREPLY(4);
    RESPONSEREPLY(5);


#define RESPONSEENABLEMACRO(NUM) ui->responseEnableCheck##NUM->setChecked(settings.value("responseEnableCheck"#NUM, false).toBool())

    RESPONSEENABLEMACRO(1);
    RESPONSEENABLEMACRO(2);
    RESPONSEENABLEMACRO(3);
    RESPONSEENABLEMACRO(4);
    RESPONSEENABLEMACRO(5);

#define ENCODEMACRO(NUM) ui->responseEncodingBox##NUM->setCurrentIndex(ui->responseEncodingBox##NUM->findText(settings.value("responseEncodingBox"#NUM,"Mixed ASCII").toString()))

    ENCODEMACRO(1);
    ENCODEMACRO(2);
    ENCODEMACRO(3);
    ENCODEMACRO(4);
    ENCODEMACRO(5);


    on_smartResponseEnableCheck_clicked();

    ui->autolaunchStarterPanelButton->setChecked(settings.value("autolaunchStarterPanelButton", false).toBool());

    ui->darkModeCheck->setChecked(settings.value("darkModeCheck", true).toBool());
    ui->httpAdjustContentTypeCheck->setChecked(settings.value("httpAdjustContentTypeCheck", true).toBool());

    ui->translateMacroSendCheck->setChecked(settings.value("translateMacroSendCheck", true).toBool());

    ui->persistentTCPCheck->setChecked(settings.value("persistentTCPCheck", false).toBool());

    ui->ignoreSSLCheck->setChecked(settings.value("ignoreSSLCheck", true).toBool());
    ui->sslCaPath->setText(settings.value("sslCaPath", "").toString());
    ui->sslLocalCertificatePath->setText(settings.value("sslLocalCertificatePath", "").toString());
    ui->sslPrivateKeyPath->setText(settings.value("sslPrivateKeyPath", "").toString());

    ui->dateFormat->setText(settings.value("dateFormat", "yyyy-MM-dd").toString());
    ui->timeFormat->setText(settings.value("timeFormat", "hh:mm:ss ap").toString());

    QDateTime now = QDateTime::currentDateTime();
    QString dateFormat = ui->dateFormat->text();
    QString timeFormat = ui->timeFormat->text();

    ui->dateFormatExample->setText(now.toString(dateFormat));
    ui->timeFormatExample->setText(now.toString(timeFormat));

    leaveSessionOpen = ui->leaveSessionOpen;
    connect(leaveSessionOpen, &QCheckBox::toggled, dynamic_cast<MainWindow*>(parent), &MainWindow::on_leaveSessionOpen_StateChanged);

    connect(ui->dateFormat, &QLineEdit::textChanged, this, [=](QString val) {
        // use action as you wish
        QDateTime now = QDateTime::currentDateTime();
        QString dateFormat = ui->dateFormat->text();
        ui->dateFormatExample->setText(now.toString(dateFormat));
    });


    connect(ui->timeFormat, &QLineEdit::textChanged, this, [=](QString val) {
        // use action as you wish
        QDateTime now = QDateTime::currentDateTime();
        QString timeFormat = ui->timeFormat->text();
        ui->timeFormatExample->setText(now.toString(timeFormat));
    });



    ui->restoreSessionCheck->setChecked(settings.value("restoreSessionCheck", true).toBool());
    ui->checkforUpdates->setChecked(settings.value("checkforUpdates", true).toBool());

    ui->resolveDNSOnInputCheck->setChecked(settings.value("resolveDNSOnInputCheck", false).toBool());

    QList<int> udpList = portsToIntList(settings.value("udpPort", "0").toString());
    QList<int> tcpList = portsToIntList(settings.value("tcpPort", "0").toString());
    QList<int> sslList = portsToIntList(settings.value("sslPort", "0").toString());

    ui->udpServerPortEdit->setText(intListToPorts(udpList));
    ui->tcpServerPortEdit->setText(intListToPorts(tcpList));
    ui->sslServerPortEdit->setText(intListToPorts(sslList));


    ui->udpServerEnableCheck->setChecked(settings.value("udpServerEnable", true).toBool());
    ui->tcpServerEnableCheck->setChecked(settings.value("tcpServerEnable", true).toBool());
    ui->sslServerEnableCheck->setChecked(settings.value("sslServerEnable", true).toBool());

    ui->serverSnakeOilCheck->setChecked(settings.value("serverSnakeOilCheck", true).toBool());

    ui->ellipsisCheck->setChecked(settings.value("ellipsisCheck", true).toBool());

    ui->attemptReceiveCheck->setChecked(settings.value("attemptReceiveCheck", false).toBool());

    ui->delayAfterConnectCheck->setChecked(settings.value("delayAfterConnectCheck", false).toBool());

    ui->rolling500entryCheck->setChecked(settings.value("rolling500entryCheck", false).toBool());
    ui->copyUnformattedCheck->setChecked(settings.value("copyUnformattedCheck", true).toBool());


    ui->sendResponseSettingsCheck->setChecked(settings.value("sendReponse", false).toBool());

    ui->hexResponseEdit->setText(settings.value("responseHex", "").toString());

    QString ascii = ui->hexResponseEdit->text();
    ui->asciiResponseEdit->setText(Packet::hexToASCII(ascii));


    QString ipMode = settings.value("ipMode", "4").toString(); //default to 4
    QDEBUGVAR(ipMode);

    ui->ipv4Radio->setChecked(false);
    ui->ipv6Radio->setChecked(false);
    ui->ipSpecificRadio->setChecked(false);

    QHostAddress h4 = QHostAddress::AnyIPv4;
    QHostAddress h6 = QHostAddress::AnyIPv6;

    bool ip4check = (ipMode == "4") || (ipMode == h4.toString());
    bool ip6check = (ipMode == "6") || (ipMode == h6.toString());

    ui->ipv4Radio->setChecked(ip4check);
    ui->ipv6Radio->setChecked(ip6check);

    if(!ip4check && !ip6check) {
        ui->ipSpecificRadio->setChecked(true);
        ui->bindIPAddress->setText(ipMode);
    }




    unsigned int resendNum = settings.value("cancelResendNum", 0).toInt();
    if (resendNum == 0) {
        ui->cancelResendNumEdit->setText("");
    } else {
        ui->cancelResendNumEdit->setText(QString::number(resendNum));
    }

    float multiSendDelay = settings.value("multiSendDelay", 0).toFloat();
    if (multiSendDelay == 0) {
        ui->multiSendDelayEdit->setText("");
    } else {
        ui->multiSendDelayEdit->setText(QString::number(multiSendDelay));
    }



    ui->settingsTabWidget->setCurrentIndex(0);

    packetsSaved = Packet::fetchAllfromDB("");
    ui->responsePacketBox->clear();
    ui->responsePacketBox->addItem("<Load..>");
    Packet tempPacket;
    foreach (tempPacket, packetsSaved) {
        ui->responsePacketBox->addItem(tempPacket.name);

    }


    setDefaultTableHeaders();
    setStoredTableHeaders();
    loadTableHeaders();

    ui->copyUnformattedCheck->setFocus();
    QDEBUG() << "Settings Loaded";

}


Settings::~Settings()
{
    delete ui;
}


bool Settings::needLanguage()
{
    //Return true if language was never chosen
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString language = settings.value("languageCombo", "").toString().toLower();

    return language.isEmpty();
}

QString Settings::language()
{

    QString locale = QLocale::system().name().section("", 0, 2);
    QDEBUGVAR(locale);
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    QString language = settings.value("languageCombo", "English").toString().toLower();
    if(language.contains("spanish")) {
        return "Spanish";
    }

    if(language.contains("german")) {
        return "German";
    }

    if(language.contains("french")) {
        return "French";
    }

    if(language.contains("italian")) {
        return "Italian";
    }

    return "English";
}


void Settings::statusBarMessage(QString msg)
{
    Q_UNUSED(msg);

}


void Settings::on_buttonBox_accepted()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QList<int> udpList = Settings::portsToIntList(ui->udpServerPortEdit->text());
    QList<int> tcpList = Settings::portsToIntList(ui->tcpServerPortEdit->text());
    QList<int> sslList = Settings::portsToIntList(ui->sslServerPortEdit->text());

    if(ui->ipSpecificRadio->isChecked()) {
        QHostAddress address(ui->bindIPAddress->text());
        if ((QAbstractSocket::IPv4Protocol == address.protocol() ) || (QAbstractSocket::IPv6Protocol == address.protocol())
                ) {
            QDEBUG() << "Binding to custom IP" << address.toString();
        } else {

            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Bad IP."));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Packet Sender cannot bind invalid IP ")+ ui->bindIPAddress->text());
            msgBox.exec();
            return;
        }
    }

    int t, s;
    foreach (t, tcpList) {
        if(t == 0) continue;
        foreach (s, sslList) {
            if(s == 0) continue;

            if(t == s) {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("TCP and SSL non-zero port conflict."));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Packet Sender cannot bind TCP and SSL to the same port."));
                msgBox.exec();
                return;
            }

        }


    }


    //settings.setValue("dtlsPort", intListToPorts(dtlsList));
    settings.setValue("udpPort", intListToPorts(udpList));
    settings.setValue("tcpPort", intListToPorts(tcpList));
    settings.setValue("sslPort", intListToPorts(sslList));

    settings.setValue("sendReponse", ui->sendResponseSettingsCheck->isChecked());
    settings.setValue("responseName", ui->responsePacketBox->currentText().trimmed());
    settings.setValue("responseHex", ui->hexResponseEdit->text().trimmed());

    //settings.setValue("dtlsServerEnable", ui->dtlsServerEnableCheck->isChecked());
    settings.setValue("udpServerEnable", ui->udpServerEnableCheck->isChecked());
    settings.setValue("tcpServerEnable", ui->tcpServerEnableCheck->isChecked());
    settings.setValue("sslServerEnable", ui->sslServerEnableCheck->isChecked());

    settings.setValue("serverSnakeOilCheck", ui->serverSnakeOilCheck->isChecked());


    settings.setValue("ellipsisCheck", ui->ellipsisCheck->isChecked());

    settings.setValue("attemptReceiveCheck", ui->attemptReceiveCheck->isChecked());

    settings.setValue("delayAfterConnectCheck", ui->delayAfterConnectCheck->isChecked());

    settings.setValue("resolveDNSOnInputCheck", ui->resolveDNSOnInputCheck->isChecked());

    settings.setValue("ignoreSSLCheck", ui->ignoreSSLCheck->isChecked());
    settings.setValue("sslCaPath", ui->sslCaPath->text());
    settings.setValue("sslLocalCertificatePath", ui->sslLocalCertificatePath->text());
    settings.setValue("sslPrivateKeyPath", ui->sslPrivateKeyPath->text());

    settings.setValue("restoreSessionCheck", ui->restoreSessionCheck->isChecked());

    settings.setValue("checkforUpdates", ui->checkforUpdates->isChecked());

    settings.setValue("dateFormat", ui->dateFormat->text());
    settings.setValue("timeFormat", ui->timeFormat->text());


    settings.setValue("copyUnformattedCheck", ui->copyUnformattedCheck->isChecked());
    settings.setValue("rolling500entryCheck", ui->rolling500entryCheck->isChecked());

    settings.setValue("rolling500entryCheck", ui->rolling500entryCheck->isChecked());

    settings.setValue("persistentTCPCheck", ui->persistentTCPCheck->isChecked());
    settings.setValue("translateMacroSendCheck", ui->translateMacroSendCheck->isChecked());

    settings.setValue("autolaunchStarterPanelButton", ui->autolaunchStarterPanelButton->isChecked());
    settings.setValue("darkModeCheck", ui->darkModeCheck->isChecked());
    settings.setValue("httpAdjustContentTypeCheck", ui->httpAdjustContentTypeCheck->isChecked());

    settings.setValue("cancelResendNum", ui->cancelResendNumEdit->text().toUInt());

    float multiSend = Packet::oneDecimal(ui->multiSendDelayEdit->text().toFloat());
    settings.setValue("multiSendDelay", multiSend);

    QString ipMode = "4";

    if (ui->ipv6Radio->isChecked()) {
        ipMode = "6";
    }

    if (ui->ipSpecificRadio->isChecked()) {
        ipMode = ui->bindIPAddress->text();
    }

    settings.setValue("ipMode", ipMode);

    //save traffic order
    QListWidget * lw = ui->displayOrderList;
    QListWidget * lwTraffic = ui->displayOrderListTraffic;
    QStringList packetSavedHeaderNow;
    packetSavedHeaderNow.clear();
    for (int i = 0; i < lw->count(); i++) {
        packetSavedHeaderNow.append(lw->item(i)->data(Qt::UserRole).toString());
    }
    settings.setValue("packetSavedTableHeaders", packetSavedHeaderNow);


    packetSavedHeaderNow.clear();
    for (int i = 0; i < lwTraffic->count(); i++) {
        packetSavedHeaderNow.append(lwTraffic->item(i)->data(Qt::UserRole).toString());
    }
    settings.setValue("packetTableHeaders", packetSavedHeaderNow);


    //smart responses...
    settings.setValue("smartResponseEnableCheck", ui->smartResponseEnableCheck->isChecked());

#define RESPONSEIFSAVE(NUM) settings.setValue("responseIfEdit" #NUM, ui->responseIfEdit##NUM->text());

    RESPONSEIFSAVE(1);
    RESPONSEIFSAVE(2);
    RESPONSEIFSAVE(3);
    RESPONSEIFSAVE(4);
    RESPONSEIFSAVE(5);

#define RESPONSEREPLYSAVE(NUM) settings.setValue("responseReplyEdit" #NUM, ui->responseReplyEdit##NUM->text());

    RESPONSEREPLYSAVE(1);
    RESPONSEREPLYSAVE(2);
    RESPONSEREPLYSAVE(3);
    RESPONSEREPLYSAVE(4);
    RESPONSEREPLYSAVE(5);


#define RESPONSEENABLEMACROSAVE(NUM) settings.setValue("responseEnableCheck" #NUM, ui->responseEnableCheck##NUM->isChecked());

    RESPONSEENABLEMACROSAVE(1);
    RESPONSEENABLEMACROSAVE(2);
    RESPONSEENABLEMACROSAVE(3);
    RESPONSEENABLEMACROSAVE(4);
    RESPONSEENABLEMACROSAVE(5);


#define ENCODEMACROSAVE(NUM) settings.setValue("responseEncodingBox" #NUM, ui->responseEncodingBox##NUM->currentText());

    ENCODEMACROSAVE(1);
    ENCODEMACROSAVE(2);
    ENCODEMACROSAVE(3);
    ENCODEMACROSAVE(4);
    ENCODEMACROSAVE(5);


}

void Settings::on_asciiResponseEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

    QString quicktestASCII =  ui->asciiResponseEdit->text();
    ui->hexResponseEdit->setText(Packet::ASCIITohex(quicktestASCII));

}

void Settings::on_hexResponseEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

    QString quicktestHex =  ui->hexResponseEdit->text();

    ui->asciiResponseEdit->setText(Packet::hexToASCII(quicktestHex));

}


QStringList Settings::defaultPacketTableHeader()
{
    QStringList list;
    list.clear();
    list << Settings::SEND_STR << Settings::NAME_STR << Settings::RESEND_STR << Settings::TOADDRESS_STR << Settings::TOPORT_STR << Settings::METHOD_STR << Settings::ASCII_STR << Settings::HEX_STR;

    return list;

}

QStringList Settings::defaultTrafficTableHeader()
{
    QStringList list;
    list.clear();
    list << Settings::TIME_STR << Settings::FROMIP_STR << Settings::FROMPORT_STR << Settings::TOADDRESS_STR << Settings::TOPORT_STR << Settings::METHOD_STR << Settings::ERROR_STR << Settings::ASCII_STR << Settings::HEX_STR;

    return list;

}

#endif

QString Settings::intListToPorts(QList<int> portList)
{
    if(portList.isEmpty()) {
        return "0";
    }

    int p;
    QStringList joinedU;

    foreach (p, portList) {
        joinedU.append(QString::number(p));
    }

    return joinedU.join(", ");

}

QList<int> Settings::portsToIntList(QString ports)
{

    QList<int> returnList;
    returnList.clear();
    QString udpPortString = ports;
    udpPortString.replace(",", " ");
    udpPortString.replace(";", " ");
    udpPortString.replace("&", " ");
    udpPortString.replace("|", " ");
    udpPortString.replace(".", " ");
    udpPortString.replace("\r", " ");
    udpPortString.replace("\n", " ");
    udpPortString = udpPortString.simplified();

    QStringList portList = udpPortString.split(" ");
    QString portS;
    bool ok = false;
    foreach (portS, portList) {
        int theport = portS.toInt(&ok);
        if(ok && (!returnList.contains(theport)) && (theport >= 0)) {
            if(theport < 65536) {
                returnList.append(theport);
            }
        }
    }

    if(returnList.isEmpty()) {
        returnList.append(0);
    }

    return returnList;


}

#ifndef CONSOLE_BUILD


void Settings::setDefaultTableHeaders()
{

    packetSavedTableHeaders = Settings::defaultPacketTableHeader();
    packetTableHeaders  = Settings::defaultTrafficTableHeader();
}

void Settings::setStoredTableHeaders()
{

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QStringList packetSavedTable = settings.value("packetSavedTableHeaders", packetSavedTableHeaders).toStringList();
    QStringList packetTable = settings.value("packetTableHeaders", packetTableHeaders).toStringList();


    QStringList originalpacketSavedTableHeaders  = Settings::defaultPacketTableHeader();
    QString saveTest;
    foreach(saveTest, packetSavedTable) {
        if(!originalpacketSavedTableHeaders.contains(saveTest)) {
            packetSavedTable = originalpacketSavedTableHeaders;
            break;
        }
    }
    if (packetSavedTable.size() == packetSavedTableHeaders.size()) {
        packetSavedTableHeaders = packetSavedTable;
    }

    if (packetTable.size() == packetTableHeaders.size()) {
        packetTableHeaders = packetTable;
    }

}

void Settings::loadTableHeaders()
{

    QString tempString;
    tempString.clear();

    QListWidgetItem * tItem;

    ui->displayOrderList->clear();
    ui->displayOrderListTraffic->clear();

    foreach (tempString, packetSavedTableHeaders) {
        tItem = new QListWidgetItem(logHeaderTranslate(tempString));
        tItem->setData(Qt::UserRole, tempString);
        tItem->setIcon(QIcon(UPDOWNICON));
        ui->displayOrderList->addItem(tItem);
    }

    ui->displayOrderList->setCursor(Qt::CrossCursor);

    foreach (tempString, packetTableHeaders) {
        tItem = new QListWidgetItem(logHeaderTranslate(tempString));
        tItem->setData(Qt::UserRole, tempString);
        tItem->setIcon(QIcon(UPDOWNICON));
        ui->displayOrderListTraffic->addItem(tItem);
    }

    ui->displayOrderListTraffic->setCursor(Qt::CrossCursor);

    ui->displayOrderList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->displayOrderList->setAlternatingRowColors(true);
    ui->displayOrderListTraffic->setDragDropMode(QAbstractItemView::InternalMove);
    ui->displayOrderListTraffic->setAlternatingRowColors(true);
}

void Settings::on_responsePacketBox_currentIndexChanged(const QString &arg1)
{

    Q_UNUSED(arg1);

    Packet tempPacket;
    QString selectedName = ui->responsePacketBox->currentText();

    //QDEBUGVAR(selectedName);
    foreach (tempPacket, packetsSaved) {
        if (tempPacket.name == selectedName) {
            ui->hexResponseEdit->setText(tempPacket.hexString);
            on_hexResponseEdit_textEdited(tempPacket.hexString);
            break;
        }

    }

    ui->responsePacketBox->setCurrentIndex(0);
}

void Settings::on_defaultDisplayButton_clicked()
{
    setDefaultTableHeaders();
    loadTableHeaders();

}


void Settings::on_smartResponseEnableCheck_clicked()
{

    ui->smartResponsesGroup->setEnabled(ui->smartResponseEnableCheck->isChecked());
}

void Settings::on_sslCaPathBrowseButton_clicked()
{

    QString home = QDir::homePath();
    if (QFile::exists(ui->sslCaPath->text())) {
        home = ui->sslCaPath->text();
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                  home,
                  QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);

    if (QFile::exists(dir)) {
        ui->sslCaPath->setText(dir);
    }


}

void Settings::on_sslLocalCertificatePathBrowseButton_clicked()
{

    QString home = QDir::homePath();
    if (QFile::exists(ui->sslLocalCertificatePath->text())) {
        home = ui->sslLocalCertificatePath->text();
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Choose Cert"), home, tr("*.*"));

    if (QFile::exists(fileName)) {
        ui->sslLocalCertificatePath->setText(fileName);
    }

}

void Settings::on_sslPrivateKeyPathBrowseButton_clicked()
{
    QString home = QDir::homePath();
    if (QFile::exists(ui->sslPrivateKeyPath->text())) {
        home = ui->sslPrivateKeyPath->text();
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Choose Key"), home, tr("*.*"));

    if (QFile::exists(fileName)) {
        ui->sslPrivateKeyPath->setText(fileName);
    }

}

void Settings::on_documentationButton_clicked()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation"));
}



void Settings::saveHTTPHeader(QString host, QString header)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QDEBUG() << "saving" << host << ":" << header;


    // Search for a duplicate and remove it.
    auto keyvalue = Settings::header2keyvalue(header);
    QStringList hostHeaders = Settings::getHTTPHeaders(host);
    for(int i=0; i<hostHeaders.size(); i++) {
        auto keyvaluecheck = Settings::header2keyvalue(hostHeaders[i]);
        if(keyvalue.first == keyvaluecheck.first) {
            hostHeaders.removeAt(i);
            break;
        }
    }
    hostHeaders.append(header);
    hostHeaders.removeDuplicates();

    settings.setValue(Settings::HTTPHEADERINDEX + host, hostHeaders);


    QStringList allHosts = settings.value(Settings::ALLHTTPSHOSTS, QStringList()).toStringList();
    allHosts.append(host);
    allHosts.removeDuplicates();
    settings.setValue(Settings::ALLHTTPSHOSTS, allHosts);

    loadCredentialTable();

}



void Settings::deleteHTTPHeader(QString host, QString header)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QDEBUG() << "removing" << host << ":" << header;

    QStringList hostHeaders = Settings::getHTTPHeaders(host);
    hostHeaders.removeDuplicates();
    int i = hostHeaders.indexOf(header);
    if(i > -1) {
        hostHeaders.removeAt(i);
        settings.setValue(Settings::HTTPHEADERINDEX + host, hostHeaders);
    }

}
#endif

QPair<QString, QString> Settings::header2keyvalue(QString header)
{

    QPair<QString, QString> keyvalue;

    QString key = "";
    QString value = "";
    QStringList headerSplit = header.split(":");
    if(headerSplit.size() > 1) {
        key = headerSplit.first();
        headerSplit.pop_front();
        value = headerSplit.join(":");
    }

    keyvalue.first = key;
    keyvalue.second = value;

    return keyvalue;

}


QHash<QString, QString> Settings::getRawHTTPHeaders(QString host)
{
    QHash<QString, QString> headers;
    QStringList headerList = Settings::getHTTPHeaders(host);
    foreach(QString header, headerList) {
        auto keyvalue = Settings::header2keyvalue(header);
        QString key = keyvalue.first;
        QString value = keyvalue.second;
        if(key.size() > 1) {
            headers[key] = value;
        }
    }
    return headers;
}

bool Settings::detectJSON_XML()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    return settings.value("httpAdjustContentTypeCheck", true).toBool();
}


QStringList Settings::getHTTPHeaders(QString host)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    return settings.value(Settings::HTTPHEADERINDEX + host, QStringList()).toStringList();
}

QHash<QString, QStringList> Settings::getAllHTTPHeaders()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QStringList allHosts = settings.value(Settings::ALLHTTPSHOSTS, QStringList()).toStringList();
    QString host;
    QHash<QString, QStringList> allHttps;
    foreach(host, allHosts) {
        allHttps[host] = Settings::getHTTPHeaders(host);
    }
    return allHttps;

}

#ifndef CONSOLE_BUILD

void Settings::clearHTTPHeaders(QString host)
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QStringList allHosts = settings.value(Settings::ALLHTTPSHOSTS, QStringList()).toStringList();
    int i = allHosts.indexOf(host);
    if(i > -1) {
        allHosts.removeAt(i);
        settings.setValue(Settings::ALLHTTPSHOSTS, allHosts);
        settings.remove(Settings::HTTPHEADERINDEX + host);
    }
}


void Settings::on_addCredentialButton_clicked()
{
    QString host = ui->httpHostEdit->text();
    QString key = ui->httpUNEdit->text();
    QString value = ui->httpPWEdit->text();

    QString header = key + ": " + value;

    if(ui->genAuthCheck->isChecked()) {
        QString b64 = QString((key+":"+value).toLatin1().toBase64());
        header = "Authorization: Basic " + b64;
        ui->genAuthCheck->setChecked(false);
        on_genAuthCheck_clicked(false);
    }
    saveHTTPHeader(host, header);

    loadCredentialTable();

}


void Settings::loadCredentialTable()
{


    httpSettingsLoading = true;
    QHash<QString, QStringList> allHttps = Settings::getAllHTTPHeaders();
    QStringList keys = allHttps.keys();
    QString key;
    ui->httpCredentialTable->setColumnCount(3);
    int row = 0;
    foreach(key, keys) {
        QString header;
        foreach(header, allHttps[key]) {
            ui->httpCredentialTable->setRowCount(row + 1);

            auto keyvalue = Settings::header2keyvalue(header);
            if(keyvalue.first.isEmpty()) continue;

            QTableWidgetItem * hostItem = new QTableWidgetItem(key);
            QTableWidgetItem * keyItem = new QTableWidgetItem(keyvalue.first);
            QTableWidgetItem * valueItem = new QTableWidgetItem(keyvalue.second);


            hostItem->setData(Qt::UserRole, key);
            hostItem->setData(Qt::UserRole + 1, header);
            hostItem->setData(Qt::UserRole + 2, "host");

            keyItem->setData(Qt::UserRole, key);
            keyItem->setData(Qt::UserRole + 1, header);
            keyItem->setData(Qt::UserRole + 2, "key");

            valueItem->setData(Qt::UserRole, key);
            valueItem->setData(Qt::UserRole + 1, header);
            valueItem->setData(Qt::UserRole + 2, "value");


            ui->httpCredentialTable->setItem(row, 0, hostItem);
            ui->httpCredentialTable->setItem(row, 1, keyItem);
            ui->httpCredentialTable->setItem(row, 2, valueItem);
            row++;
        }
    }

    QStringList tableHeaders = {tr("Host"), tr("Key"), tr("Value")};
    ui->httpCredentialTable->verticalHeader()->show();
    ui->httpCredentialTable->horizontalHeader()->show();
    ui->httpCredentialTable->setHorizontalHeaderLabels(tableHeaders);
    ui->httpCredentialTable->resizeColumnsToContents();
    ui->httpCredentialTable->resizeRowsToContents();
    ui->httpCredentialTable->horizontalHeader()->setStretchLastSection(true);

    httpSettingsLoading = false;
}




void Settings::on_httpDeleteHeaderButton_clicked()
{

    QList<QTableWidgetItem *> totalSelected  = ui->httpCredentialTable->selectedItems();
    QTableWidgetItem * item;
    foreach (item, totalSelected) {
        QString host = item->data(Qt::UserRole).toString();
        QString header = item->data(Qt::UserRole + 1).toString();
        deleteHTTPHeader(host, header);
    }
    ui->httpCredentialTable->clearSelection();
    loadCredentialTable();
}


void Settings::on_httpCredentialTable_itemChanged(QTableWidgetItem *item)
{

    if(httpSettingsLoading) return;

    QString newText = item->text();
    QDEBUGVAR(newText);

    QString index = item->data(Qt::UserRole + 2).toString();
    QString host = item->data(Qt::UserRole).toString();
    QString header = item->data(Qt::UserRole + 1).toString();
    auto keyvalue = Settings::header2keyvalue(header);

    if(index == "host") {
        deleteHTTPHeader(host, header);
        saveHTTPHeader(newText, header);
    }

    if(index == "key") {
        deleteHTTPHeader(host, header);
        saveHTTPHeader(host, newText + ": " + keyvalue.second);
    }

    if(index == "value") {
        deleteHTTPHeader(host, header);
        saveHTTPHeader(host, keyvalue.first + ": " + newText);
    }

    ui->httpCredentialTable->clearSelection();
    loadCredentialTable();

}

void Settings::on_genAuthCheck_clicked(bool checked)
{
    if(ui->genAuthCheck->isChecked()) {
        ui->httpKeyLabel->setText(tr("UN/ClientID"));
        ui->httpValueLabel->setText(tr("PW/Access"));
        ui->addCredentialButton->setText(tr("HTTP Auth Header"));
    } else {
        ui->httpKeyLabel->setText(tr("Key"));
        ui->httpValueLabel->setText(tr("Value"));
        ui->addCredentialButton->setText(tr("HTTP Header"));
    }

}
#endif

void Settings::on_chooseLanguageButton_clicked()
{

#ifndef CONSOLE_BUILD
    LanguageChooser * lang = new LanguageChooser(this);
    lang->exec();

#endif

}

