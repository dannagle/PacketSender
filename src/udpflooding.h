#ifndef UDPFLOODING_H
#define UDPFLOODING_H

#include <QDialog>
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QtNetwork/QUdpSocket>

class ThreadSender : public QThread
{

    public:
        ThreadSender(QObject *parent);
        ~ThreadSender();
        void run();
        double getRatekHz(QElapsedTimer eTimer, quint64 pkts);
        qint64  getElapsedMS();
        QString ip;
        quint16 port;
        int delay;
        bool speedSendEnabled;
        QString ascii;
        QByteArray hex;
        bool issending;
        bool stopsending;
        quint64 packetssent;
        unsigned int sourcePort;
        QElapsedTimer elapsedTimer;

        int short_burst_of(int, QUdpSocket*, QHostAddress*);
};


namespace Ui {
class UDPFlooding;
}



class UDPFlooding : public QDialog
{
        Q_OBJECT

    public:
        explicit UDPFlooding(QWidget *parent = nullptr, QString target = "192.168.1.1", quint16 port = 5000, QString ascii = "data");
        ~UDPFlooding();

    private slots:
        void on_startButton_clicked();

        void on_stopButton_clicked();

        void refreshTimerTimeout();

    signals:
        void operate(const QString &);

    private:
        Ui::UDPFlooding *ui;
        ThreadSender *thread;
        QTimer refreshTimer;
};

#endif // UDPFLOODING_H
