#pragma once


#include <QDialog>
#include <QStringList>

namespace Ui {
class PersistentHTTP;
}

class PersistentHTTP : public QDialog
{
    Q_OBJECT

public:
    explicit PersistentHTTP(QWidget *parent = nullptr);
    void init(QByteArray thedata, QUrl url);
    ~PersistentHTTP();

private slots:
    void on_copyCodeButton_clicked();



    void on_browserViewButton_clicked();

private:
    Ui::PersistentHTTP *ui;
    QByteArray data;
    QStringList tempFiles;


};


