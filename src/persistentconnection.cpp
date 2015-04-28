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
    refreshTimer.setInterval(2000);
    refreshTimer.start();
    trafficList.clear();

}

void PersistentConnection::aboutToClose() {
    QDEBUG() << "Stopping timer";
    refreshTimer.stop();
    QDEBUG() << "Stopping thread";
    thread->terminate();

}

void PersistentConnection::statusReceiver(QString message)
{
    QDEBUGVAR(message);
    ui->topLabel->setText(message);

    if(message.toLower().startsWith("not connected")) {

        QDEBUG() << "Setting style sheet";
        ui->trafficViewEdit->setStyleSheet("QTextEdit { background-color: #EEEEEE }");
        ui->asciiSendButton->setEnabled(false);
        ui->asciiLineEdit->setEnabled(false);
    }


}


PersistentConnection::~PersistentConnection()
{
    aboutToClose();
    delete ui;
}


/*
void PersistentConnection::reject()
{
    QDEBUG() << "Stopping timer";
    refreshTimer.stop();

}
*/

void PersistentConnection::init() {

    QDEBUG();
    setWindowTitle("TCP://"+sendPacket.toIP + ":" + QString::number(sendPacket.port));
    QDEBUG();

    thread = new TCPThread(sendPacket, this);

    QDEBUG() << ": thread Connection attempt " <<
                connect ( this , SIGNAL ( persistentPacketSend(Packet) ) , thread, SLOT ( sendPersistant(Packet) ) )
             << connect ( thread , SIGNAL ( connectStatus(QString) ) , this, SLOT ( statusReceiver(QString) ) )
             << connect ( thread , SIGNAL ( packetSent(Packet)), this, SLOT(packetSentSlot(Packet)));
                ;



    QApplication::processEvents();

    thread->start();


    QApplication::processEvents();

}

void PersistentConnection::packetToSend(Packet sendpacket)
{
    QDEBUG();

}

void PersistentConnection::refreshTimerTimeout() {
    QDEBUG();

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

    emit persistentPacketSend(asciiPacket);
    ui->asciiLineEdit->setText("");

}
