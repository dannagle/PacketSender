#pragma once

#ifdef CHROMIUM

#include <QDialog>

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

    void on_copyRenderButton_clicked();

private:
    Ui::PersistentHTTP *ui;
    QByteArray data;
};

#endif
