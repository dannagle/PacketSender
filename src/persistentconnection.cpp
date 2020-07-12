#include "persistentconnection.h"
#include "ui_persistentconnection.h"

#include <QTextStream>
#include <QScrollBar>
#include <QSettings>
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QShortcut>
#include <QCompleter>
#include <QStringList>
#include <QFileDialog>
#include <QClipboard>
#include <QMessageBox>


PersistentConnection::PersistentConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersistentConnection)
{
    ui->setupUi(this);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(this->windowFlags() | Qt::WindowMaximizeButtonHint);

    suppressSlot = true;
    previousCommands.clear();



    QDEBUG();
    sendPacket.clear();
    QDEBUG() << ": refreshTimer Connection attempt " <<
             connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout()))
             << connect(this, SIGNAL(rejected()), this, SLOT(aboutToClose()))
             << connect(this, SIGNAL(accepted()), this, SLOT(aboutToClose()))
             << connect(this, SIGNAL(dialogIsClosing()), this, SLOT(aboutToClose()));
    QDEBUG() << "Setup timer";
    refreshTimer.setInterval(200);
    refreshTimer.start();
    trafficList.clear();
    startTime = QDateTime::currentDateTime();
    wasConnected = false;
    stopTimer = false;

    ui->searchEdit->setText("");

    loadComboBox();

    useraw = true;

    ui->asciiLineEdit->setFocus();
    suppressSlot = false;


    QFont font("monospace");

#ifdef __APPLE__
    font.setStyleHint(QFont::Monospace);
#else
    font.setStyleHint(QFont::TypeWriter);
#endif
    ui->trafficViewEdit->setFont(font);

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    translateMacroSend = settings.value("translateMacroSendCheck", true).toBool();
}


void PersistentConnection::loadComboBox()
{

    QList<Packet> packetsSaved = Packet::fetchAllfromDB("");
    ui->packetComboBox->clear();
    Packet tempPacket;
    QString search = ui->searchEdit->text().trimmed().toLower();
    foreach (tempPacket, packetsSaved) {
        if (tempPacket.name.trimmed().toLower().contains(search)) {
            ui->packetComboBox->addItem(tempPacket.name);
        }

    }

}

void PersistentConnection::aboutToClose()
{
    QDEBUG() << "Stopping timer";
    refreshTimer.stop();
    QDEBUG() << "checking thread null";
    if (thread == NULL) {
        QDEBUG() << "pointer is null";
    } else {
        QDEBUG() << "requesting stop";
        thread->closeRequest = true;
    }

    //cannot reliably call "wait" on a thread, so just exit.

}

void PersistentConnection::statusReceiver(QString message)
{
    //QDEBUGVAR(message);
    ui->topLabel->setText(message);

    if (message.startsWith("Connected")) {
        wasConnected = true;
    }

    if (message.toLower().startsWith("not connected")) {

        QDEBUG() << "Setting style sheet";

        ui->trafficViewEdit->setStyleSheet("QTextEdit { background-color: #000 }");
        ui->asciiSendButton->setEnabled(false);
        ui->asciiLineEdit->setEnabled(false);
        ui->packetComboBox->setEnabled(false);
        ui->appendCRcheck->setEnabled(false);
        ui->searchEdit->setEnabled(false);
        ui->packetComboBox->setEnabled(false);
        ui->LoadButton->setEnabled(false);

        ui->stopResendingButton->hide();
        stopTimer = true;
    }


}


PersistentConnection::~PersistentConnection()
{
    delete ui;
}



void PersistentConnection::initWithThread(TCPThread * thethread, quint16 portNum)
{

    thread = thethread;

    if (thread->isSecure) {
        setWindowTitle("SSL://You:" + QString::number(portNum));
    } else {
        setWindowTitle("TCP://You:" + QString::number(portNum));
    }

    QApplication::processEvents();

    ui->stopResendingButton->hide();

    QApplication::processEvents();
}


void PersistentConnection::init()
{

    this->thread = nullptr;
    QString tcpOrSSL = "TCP";
    if (sendPacket.isSSL()) {
        tcpOrSSL = "SSL";
    }
    setWindowTitle(tcpOrSSL + "://" + sendPacket.toIP + ":" + QString::number(sendPacket.port));


    reSendPacket.clear();
    if (sendPacket.repeat > 0) {
        QDEBUG() << "This packet is repeating";
        reSendPacket = sendPacket;
    } else {

        ui->stopResendingButton->hide();
    }


    QApplication::processEvents();


    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    bool appendCR = settings.value("appendCRcheck", true).toBool();
    ui->appendCRcheck->setChecked(appendCR);


    ui->stopResendingButton->setStyleSheet("QPushButton { color: black; } QPushButton::hover { color: #BC810C; } ");
    ui->stopResendingButton->setFlat(true);
    ui->stopResendingButton->setCursor(Qt::PointingHandCursor);
    ui->stopResendingButton->setIcon(QIcon(PSLOGO));

    connect(ui->stopResendingButton, &QPushButton::clicked, this, &PersistentConnection::cancelResends);

}


void PersistentConnection::cancelResends()
{
    QDEBUG();
    ui->stopResendingButton->hide();
    reSendPacket.clear();
}


void PersistentConnection::refreshTimerTimeout()
{
//    QDEBUG();

    qint64 diff = startTime.msecsTo(QDateTime::currentDateTime());

    if (thread->isRunning() && !thread->closeRequest) {
        QString winTitle = windowTitle();
        if (winTitle.startsWith("TCP://") && thread->isEncrypted()) {
            winTitle.replace("TCP://", "SSL://");
            setWindowTitle(winTitle);
        }
    }


    qint64 hours = diff / (1000 * 60 * 60);
    qint64 diffRem = diff - hours * (1000 * 60 * 60);
    qint64 min = diffRem / (1000 * 60);
    diffRem = diffRem - min * (1000 * 60);
    qint64 sec = diffRem / (1000);

    QString datestamp = QString("%1:%2:%3")
                        .arg(hours, 2, 10, QChar('0'))
                        .arg(min, 2, 10, QChar('0'))
                        .arg(sec, 2, 10, QChar('0'));

    if (wasConnected && !stopTimer) {

        ui->timeLabel->setText(datestamp);

        QDateTime now = QDateTime::currentDateTime();
        int repeatMS = (int)(reSendPacket.repeat * 1000 - 100);
        if (reSendPacket.timestamp.addMSecs(repeatMS) < now) {
            reSendPacket.timestamp = now;
            emit persistentPacketSend(reSendPacket);
        }

    }








//    QDEBUG() <<"Diff:" << diff;


}

void PersistentConnection::on_buttonBox_rejected()
{

    QDEBUG() << "Stopping timer";
    refreshTimer.stop();

}

void PersistentConnection::loadTrafficView()
{


    QDEBUGVAR(trafficList.size());

    Packet loopPkt;

    QString html;
    html.clear();
    QTextStream out(&html);

    //        clipboard->setText(QString(savePacket.getByteArray()));

    if (useraw) {
        foreach (loopPkt, trafficList) {
            out << QString(loopPkt.getByteArray());
        }

        ui->trafficViewEdit->setPlainText(html);

    } else {



        out << "<html>" << "<b>" << trafficList.size() << " packets." << "</b><br>";

        int count = 0;
        foreach (loopPkt, trafficList) {
            QDEBUG() << "Packet Loop:" << count++ << loopPkt.asciiString();
            if (loopPkt.fromIP.toLower() == "you") {
                out << "<p style='color:lightgreen'>";
            } else {
                out << "<p>";
            }
            out << loopPkt.asciiString().toHtmlEscaped();
            out << "</p>";
        }

        out << "</html>";
        ui->trafficViewEdit->setHtml(html);


    }



    QScrollBar *vScrollBar = ui->trafficViewEdit->verticalScrollBar();
    vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
    QApplication::processEvents();

}

void PersistentConnection::packetSentSlot(Packet pkt)
{

    QDEBUGVAR(pkt.hexString.size());
    trafficList.append(pkt);
    loadTrafficView();

}

void PersistentConnection::packetReceivedSlot(Packet pkt)
{
    QDEBUGVAR(pkt.hexString.size());
    trafficList.append(pkt);
    loadTrafficView();
}

void PersistentConnection::socketDisconnected()
{
    statusReceiver("not connected");
}

void PersistentConnection::on_asciiSendButton_clicked()
{
    QString ascii = ui->asciiLineEdit->text();
    if (ascii.isEmpty()) {
        return;
    }
    Packet asciiPacket;
    asciiPacket.clear();

    asciiPacket.tcpOrUdp = sendPacket.tcpOrUdp;
    asciiPacket.fromIP = "You";
    asciiPacket.toIP = sendPacket.toIP;
    asciiPacket.port = sendPacket.port;
    asciiPacket.hexString = Packet::ASCIITohex(ascii);

    if(translateMacroSend) {
        QString data = Packet::macroSwap(asciiPacket.asciiString());
        asciiPacket.hexString = Packet::ASCIITohex(data);
    }
    previousCommands.append(ascii);
    previousCommands.removeDuplicates();

    QCompleter* completer = new QCompleter(previousCommands);

    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->asciiLineEdit->setCompleter(completer);



    QDEBUGVAR(asciiPacket.hexString);
    if (ui->appendCRcheck->isChecked()) {
        asciiPacket.hexString.append(" 0d");
    }


    asciiPacket.receiveBeforeSend = false;

    emit persistentPacketSend(asciiPacket);
    ui->asciiLineEdit->setText("");

}

void PersistentConnection::on_packetComboBox_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);


}

void PersistentConnection::on_searchEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    bool suppressSlotSave = suppressSlot;
    suppressSlot = true;
    loadComboBox();
    suppressSlot = suppressSlotSave;

}

void PersistentConnection::on_asciiCheck_clicked(bool checked)
{
    if (checked) {
        useraw = false;
    }
    loadTrafficView();


}

void PersistentConnection::on_rawCheck_clicked(bool checked)
{
    if (checked) {
        useraw = true;
    }
    loadTrafficView();


}

void PersistentConnection::on_LoadButton_clicked()
{
    Packet tempPacket;
    QString selectedName = ui->packetComboBox->currentText();
    QList<Packet> packetsSaved = Packet::fetchAllfromDB("");


    //QDEBUGVAR(selectedName);
    foreach (tempPacket, packetsSaved) {
        if (tempPacket.name == selectedName) {
            ui->asciiLineEdit->setText(tempPacket.asciiString());
            break;
        }

    }
}

void PersistentConnection::on_packetComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!suppressSlot) {
        on_LoadButton_clicked();
    }

}

void PersistentConnection::on_clearButton_clicked()
{
    trafficList.clear();
    loadTrafficView();

}

void PersistentConnection::on_sendFileButton_clicked()
{

    static QString fileName;

    if (fileName.isEmpty()) {
        fileName = QDir::homePath();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Send File"),
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
    }

    Packet asciiPacket;
    asciiPacket.clear();

    asciiPacket.tcpOrUdp = sendPacket.tcpOrUdp;
    asciiPacket.fromIP = "You";
    asciiPacket.toIP = sendPacket.toIP;
    asciiPacket.port = sendPacket.port;
    asciiPacket.hexString = Packet::byteArrayToHex(data);

    asciiPacket.receiveBeforeSend = false;

    emit persistentPacketSend(asciiPacket);
}

void PersistentConnection::on_clipboardButton_clicked()
{

    QClipboard *clipboard = QApplication::clipboard();

    clipboard->setText(ui->trafficViewEdit->toPlainText());
    QMessageBox msgbox;
    msgbox.setWindowTitle("Copied");
    msgbox.setText("Output sent to your clipboard");
    msgbox.exec();

}

void PersistentConnection::on_appendCRcheck_clicked()
{
    bool appendCR = ui->appendCRcheck->isChecked();
    QDEBUGVAR(appendCR);
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("appendCRcheck", appendCR);
}
