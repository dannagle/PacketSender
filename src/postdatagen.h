#pragma once

#include <QDialog>
#include <QMap>

namespace Ui {
class PostDataGen;
}

class PostDataGen : public QDialog
{
    Q_OBJECT

public:
    explicit PostDataGen(QWidget *parent = nullptr, QString query = "");
    ~PostDataGen();

signals:
    void postGenerated(QString data);

private slots:
    void on_buttonBox_accepted();
    void removeParam();
    void editParam();


    void on_addParamButton_clicked();

private:
    Ui::PostDataGen *ui;
    QList<QPair<QString, QString>> paramList;

    void generateParamUI();
};

