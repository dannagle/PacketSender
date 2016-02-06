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


    //smart responses...

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

    ui->udpServerPortEdit->setText(settings.value("udpPort","55056").toString());
    ui->tcpServerPortEdit->setText(settings.value("tcpPort","55056").toString());


    ui->udpServerEnableCheck->setChecked(settings.value("udpServerEnable", true).toBool());
    ui->tcpServerEnableCheck->setChecked(settings.value("tcpServerEnable", true).toBool());
    ui->attemptReceiveCheck->setChecked(settings.value("attemptReceiveCheck", false).toBool());

    ui->delayAfterConnectCheck->setChecked(settings.value("delayAfterConnectCheck", false).toBool());

    ui->rolling500entryCheck->setChecked(settings.value("rolling500entryCheck", false).toBool());
    ui->copyUnformattedCheck->setChecked(settings.value("copyUnformattedCheck", false).toBool());


    ui->sendResponseSettingsCheck->setChecked(settings.value("sendReponse", false).toBool());

    ui->hexResponseEdit->setText(settings.value("responseHex","").toString());

    int ipMode = settings.value("ipMode", 4).toInt();
    if (ipMode > 4) {
        ui->ipv6Radio->setChecked(true);
        ui->ipv4Radio->setChecked(false);
    } else {
        ui->ipv6Radio->setChecked(false);
        ui->ipv4Radio->setChecked(true);
    }



    unsigned int resendNum = settings.value("cancelResendNum", 0).toInt();
    if(resendNum == 0) {
        ui->cancelResendNumEdit->setText("");
    } else {
        ui->cancelResendNumEdit->setText(QString::number(resendNum));
    }

    float multiSendDelay = settings.value("multiSendDelay", 0).toFloat();
    if(multiSendDelay == 0) {
        ui->multiSendDelayEdit->setText("");
    } else {
        ui->multiSendDelayEdit->setText(QString::number(multiSendDelay));
    }



    ui->settingsTabWidget->setCurrentIndex(0);

    packetsSaved = Packet::fetchAllfromDB("");
    ui->responsePacketBox->clear();
    ui->responsePacketBox->addItem("<Load..>");
    Packet tempPacket;
    foreach(tempPacket, packetsSaved)
    {
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

    settings.setValue("udpPort", ui->udpServerPortEdit->text().toUInt());
    settings.setValue("tcpPort", ui->tcpServerPortEdit->text().toUInt());
    settings.setValue("sendReponse", ui->sendResponseSettingsCheck->isChecked());
    settings.setValue("responseName", ui->responsePacketBox->currentText().trimmed());
    settings.setValue("responseHex", ui->hexResponseEdit->text().trimmed());

    settings.setValue("udpServerEnable", ui->udpServerEnableCheck->isChecked());
    settings.setValue("tcpServerEnable", ui->tcpServerEnableCheck->isChecked());
    settings.setValue("attemptReceiveCheck", ui->attemptReceiveCheck->isChecked());

    settings.setValue("delayAfterConnectCheck", ui->delayAfterConnectCheck->isChecked());

    settings.setValue("copyUnformattedCheck", ui->copyUnformattedCheck->isChecked());
    settings.setValue("rolling500entryCheck", ui->rolling500entryCheck->isChecked());

    settings.setValue("rolling500entryCheck", ui->rolling500entryCheck->isChecked());


    settings.setValue("cancelResendNum", ui->cancelResendNumEdit->text().toUInt());

    float multiSend = Packet::oneDecimal(ui->multiSendDelayEdit->text().toFloat());
    settings.setValue("multiSendDelay", multiSend);

    if(ui->ipv6Radio->isChecked()) {
        settings.setValue("ipMode", 6);
    }

    if(ui->ipv4Radio->isChecked()) {
        settings.setValue("ipMode", 4);
    }


    //save traffic order
    QListWidget * lw = ui->displayOrderList;
    QListWidget * lwTraffic = ui->displayOrderListTraffic;
    QStringList packetSavedHeaderNow;
    packetSavedHeaderNow.clear();
    for(int i = 0; i < lw->count(); i++)
    {
        packetSavedHeaderNow.append(lw->item(i)->text());
    }
    settings.setValue("packetSavedTableHeaders", packetSavedHeaderNow);


    packetSavedHeaderNow.clear();
    for(int i = 0; i < lwTraffic->count(); i++)
    {
        packetSavedHeaderNow.append(lwTraffic->item(i)->text());
    }
    settings.setValue("packetTableHeaders", packetSavedHeaderNow);


    //smart responses...


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
    ui->hexResponseEdit->setText( Packet::ASCIITohex(quicktestASCII));

}

void Settings::on_hexResponseEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

    QString quicktestHex =  ui->hexResponseEdit->text();

    ui->asciiResponseEdit->setText(Packet::hexToASCII(quicktestHex));

}


QStringList Settings::defaultPacketTableHeader() {
    QStringList list;
    list.clear();
    list <<"Send" << "Name"<<"Resend (sec)" << "To Address" << "To Port" << "Method" << "ASCII" << "Hex";

    return list;

}

QStringList Settings::defaultTrafficTableHeader() {
    QStringList list;
    list.clear();
    list <<"Time" << "From IP" <<"From Port" << "To IP" << "To Port" << "Method"<<"Error" << "ASCII" << "Hex";

    return list;

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


    if(packetSavedTable.size() == packetSavedTableHeaders.size()) {
        packetSavedTableHeaders = packetSavedTable;
    }

    if(packetTable.size() == packetTableHeaders.size()) {
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


void Settings::on_responsePacketBox_currentIndexChanged(const QString &arg1)
{

    Q_UNUSED(arg1);

    Packet tempPacket;
    QString selectedName = ui->responsePacketBox->currentText();

    //QDEBUGVAR(selectedName);
    foreach(tempPacket, packetsSaved)
    {
        if(tempPacket.name == selectedName)
        {
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

