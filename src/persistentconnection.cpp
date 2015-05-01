#include "persistentconnection.h"
#include "ui_persistentconnection.h"

#include <QTextStream>
#include <QScrollBar>

PersistentConnection::PersistentConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersistentConnection)
{
    ui->setupUi(this);
    QDEBUG();
    sendPacket.clear();
    QDEBUG() << ": refreshTimer Connection attempt " <<
                connect ( &refreshTimer , SIGNAL ( timeout() ) , this, SLOT ( refreshTimerTimeout (  ) ) )
             << connect ( this , SIGNAL ( rejected() ) , this, SLOT ( aboutToClose (  ) ) )
             << connect ( this , SIGNAL ( accepted() ) , this, SLOT ( aboutToClose (  ) ) )
             << connect ( this , SIGNAL ( dialogIsClosing() ) , this, SLOT ( aboutToClose (  ) ) );
    QDEBUG() << "Setup timer";
    refreshTimer.setInterval(200);
    refreshTimer.start();
    trafficList.clear();
    startTime = QDateTime::currentDateTime();
    wasConnected = false;

}


void PersistentConnection::aboutToClose() {
    QDEBUG() << "Stopping timer";
    refreshTimer.stop();
    thread->terminate();
}

void PersistentConnection::statusReceiver(QString message)
{
    QDEBUGVAR(message);
    ui->topLabel->setText(message);

    if(message.startsWith("Connected")) {
        wasConnected = true;
    }

    if(message.toLower().startsWith("not connected")) {

        QDEBUG() << "Setting style sheet";
        ui->trafficViewEdit->setStyleSheet("QTextEdit { background-color: #EEEEEE }");
        ui->asciiSendButton->setEnabled(false);
        ui->asciiLineEdit->setEnabled(false);
    }


}


PersistentConnection::~PersistentConnection()
{
    delete ui;
}



void PersistentConnection::init() {

    QDEBUG();
    setWindowTitle("TCP://"+sendPacket.toIP + ":" + QString::number(sendPacket.port));
    QDEBUG();

    thread = new TCPThread(sendPacket, this);

    QDEBUG() << ": thread Connection attempt " <<
                connect ( this , SIGNAL ( persistentPacketSend(Packet) ) , thread, SLOT ( sendPersistant(Packet) ) )
             << connect ( this , SIGNAL ( closeConnection() ) , thread, SLOT ( closeConnection() ) )
             << connect ( thread , SIGNAL ( connectStatus(QString) ) , this, SLOT ( statusReceiver(QString) ) )
             << connect ( thread , SIGNAL ( packetSent(Packet)), this, SLOT(packetSentSlot(Packet)));

    QApplication::processEvents();

    thread->start();


    QApplication::processEvents();

}

void PersistentConnection::packetToSend(Packet sendpacket)
{
    QDEBUG();

}

void PersistentConnection::refreshTimerTimeout() {
//    QDEBUG();

    qint64 diff = startTime.msecsTo(QDateTime::currentDateTime());


    qint64 hours = diff / (1000 * 60 * 60);
    qint64 diffRem = diff - hours * (1000 * 60 * 60);
    qint64 min = diffRem / (1000 * 60);
    diffRem = diffRem - min * (1000 * 60);
    qint64 sec = diffRem / (1000);

    QString datestamp = QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(min, 2, 10, QChar('0'))
            .arg(sec, 2, 10, QChar('0'));

    ui->timeLabel->setText(datestamp);



//    QDEBUG() <<"Diff:" << diff;


}

void PersistentConnection::on_buttonBox_rejected()
{

    QDEBUG() << "Stopping timer";
    refreshTimer.stop();

}


void PersistentConnection::packetSentSlot(Packet pkt) {

    Packet loopPkt = pkt;
    trafficList.append(loopPkt);
    QString html;
    html.clear();
    QTextStream out(&html);
    out << "<html>" << "<b>" << trafficList.size() << " packets." << "</b><br>";

    int count = 0;
    foreach(loopPkt, trafficList) {
        QDEBUG() << "Packet Loop:" << count++ << loopPkt.asciiString();
        if(loopPkt.fromIP.toLower() == "you") {
            out << "<p style='color:blue'>";
        } else {
            out << "<p>";
        }
        out << loopPkt.asciiString().toHtmlEscaped();
        out << "</p>";
    }

    out << "</html>";
    ui->trafficViewEdit->setHtml(html);


    QScrollBar *vScrollBar = ui->trafficViewEdit->verticalScrollBar();
    vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
    QApplication::processEvents();

}

void PersistentConnection::on_asciiSendButton_clicked()
{
    QString ascii = ui->asciiLineEdit->text();
    if(ascii.isEmpty()) {
        return;
    }
    Packet asciiPacket;
    asciiPacket.clear();

    asciiPacket.tcpOrUdp = "TCP";
    asciiPacket.fromIP = "You";
    asciiPacket.toIP = sendPacket.toIP;
    asciiPacket.port = sendPacket.port;
    asciiPacket.hexString = Packet::ASCIITohex(ascii);

    QDEBUGVAR(asciiPacket.hexString);
    if(ui->appendCRcheck->isChecked()) {
        asciiPacket.hexString.append(" 0d");
    }


    asciiPacket.receiveBeforeSend = false;

    emit persistentPacketSend(asciiPacket);
    ui->asciiLineEdit->setText("");

}
