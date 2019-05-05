#ifndef UDPFLOODING_H
#define UDPFLOODING_H

#include <QDialog>
#include <QThread>

namespace Ui {
class UDPFlooding;
}


class ThreadSender : public QThread
{

public:
    ThreadSender(QObject *parent);
    void run();
    QString ip;
    unsigned int port;
    unsigned long rate;
    QString ascii;
    bool startsending;
    bool stopsending;

};


class UDPFlooding : public QDialog
{
    Q_OBJECT

public:
    explicit UDPFlooding(QWidget *parent = nullptr);
    ~UDPFlooding();

private slots:
    void on_startButton_clicked();

    void on_stopButton_clicked();

signals:
    void operate(const QString &);

private:
    Ui::UDPFlooding *ui;

    void sendThread();
    ThreadSender *thread;
};

#endif // UDPFLOODING_H
