#ifndef UDPFLOODING_H
#define UDPFLOODING_H

#include <QDialog>
#include <QThread>
#include <QDateTime>
#include <QTimer>

class ThreadSender : public QThread
{

public:
    ThreadSender(QObject *parent);
    void run();
    QString ip;
    unsigned int port;
    unsigned long rate;
    QString ascii;
    bool issending;
    bool stopsending;
    quint64 packetssent;
    QDateTime starttime;


};


namespace Ui {
class UDPFlooding;
}



class UDPFlooding : public QDialog
{
    Q_OBJECT

public:
    explicit UDPFlooding(QWidget *parent = nullptr);
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
