#include "settings.h"
#include "ui_settings.h"

#include <QStringList>
#include <QDebug>
#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QPair>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QHostAddress>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);


    //not working yet...
    ui->multiSendDelayLabel->hide();
    ui->multiSendDelayEdit->hide();


    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    QIcon mIcon(":pslogo.png");
    setWindowTitle("Packet Sender Settings");
    setWindowIcon(mIcon);

    //this is no longer working thanks to faster traffic log
    ui->displayOrderListTraffic->hide();
    ui->displayGroupBoxTraffic->setTitle("");


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


    ui->translateMacroSendCheck->setChecked(settings.value("translateMacroSendCheck", true).toBool());

    ui->persistentTCPCheck->setChecked(settings.value("persistentTCPCheck", false).toBool());

    ui->ignoreSSLCheck->setChecked(settings.value("ignoreSSLCheck", true).toBool());
    ui->sslCaPath->setText(settings.value("sslCaPath", "").toString());
    ui->sslLocalCertificatePath->setText(settings.value("sslLocalCertificatePath", "").toString());
    ui->sslPrivateKeyPath->setText(settings.value("sslPrivateKeyPath", "").toString());

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
            msgBox.setWindowTitle("Bad IP.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Packet Sender cannot bind invalid IP "+ ui->bindIPAddress->text());
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
                msgBox.setWindowTitle("TCP and SSL non-zero port conflict.");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Packet Sender cannot bind TCP and SSL to the same port.");
                msgBox.exec();
                return;
            }

        }


    }



    settings.setValue("udpPort", intListToPorts(udpList));
    settings.setValue("tcpPort", intListToPorts(tcpList));
    settings.setValue("sslPort", intListToPorts(sslList));

    settings.setValue("sendReponse", ui->sendResponseSettingsCheck->isChecked());
    settings.setValue("responseName", ui->responsePacketBox->currentText().trimmed());
    settings.setValue("responseHex", ui->hexResponseEdit->text().trimmed());

    settings.setValue("udpServerEnable", ui->udpServerEnableCheck->isChecked());
    settings.setValue("tcpServerEnable", ui->tcpServerEnableCheck->isChecked());
    settings.setValue("sslServerEnable", ui->sslServerEnableCheck->isChecked());

    settings.setValue("serverSnakeOilCheck", ui->serverSnakeOilCheck->isChecked());


    settings.setValue("attemptReceiveCheck", ui->attemptReceiveCheck->isChecked());

    settings.setValue("delayAfterConnectCheck", ui->delayAfterConnectCheck->isChecked());

    settings.setValue("resolveDNSOnInputCheck", ui->resolveDNSOnInputCheck->isChecked());

    settings.setValue("ignoreSSLCheck", ui->ignoreSSLCheck->isChecked());
    settings.setValue("sslCaPath", ui->sslCaPath->text());
    settings.setValue("sslLocalCertificatePath", ui->sslLocalCertificatePath->text());
    settings.setValue("sslPrivateKeyPath", ui->sslPrivateKeyPath->text());

    settings.setValue("restoreSessionCheck", ui->restoreSessionCheck->isChecked());

    settings.setValue("checkforUpdates", ui->checkforUpdates->isChecked());


    settings.setValue("copyUnformattedCheck", ui->copyUnformattedCheck->isChecked());
    settings.setValue("rolling500entryCheck", ui->rolling500entryCheck->isChecked());

    settings.setValue("rolling500entryCheck", ui->rolling500entryCheck->isChecked());

    settings.setValue("persistentTCPCheck", ui->persistentTCPCheck->isChecked());

    settings.setValue("translateMacroSendCheck", ui->translateMacroSendCheck->isChecked());


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
        packetSavedHeaderNow.append(lw->item(i)->text());
    }
    settings.setValue("packetSavedTableHeaders", packetSavedHeaderNow);


    packetSavedHeaderNow.clear();
    for (int i = 0; i < lwTraffic->count(); i++) {
        packetSavedHeaderNow.append(lwTraffic->item(i)->text());
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
    list << "Send" << "Name" << "Resend (sec)" << "To Address" << "To Port" << "Method" << "ASCII" << "Hex";

    return list;

}

QStringList Settings::defaultTrafficTableHeader()
{
    QStringList list;
    list.clear();
    list << "Time" << "From IP" << "From Port" << "To IP" << "To Port" << "Method" << "Error" << "ASCII" << "Hex";

    return list;

}

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
        tItem = new QListWidgetItem(tempString);
        tItem->setIcon(QIcon(UPDOWNICON));
        ui->displayOrderList->addItem(tItem);
    }

    ui->displayOrderList->setCursor(Qt::CrossCursor);

    foreach (tempString, packetTableHeaders) {
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
                       tr("Choose Cert"), home, tr("Certs (*.pem)"));

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
                       tr("Choose Key"), home, tr("Keys (*.key, *.pem)"));

    if (QFile::exists(fileName)) {
        ui->sslPrivateKeyPath->setText(fileName);
    }

}

void Settings::on_documentationButton_clicked()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation"));
}
