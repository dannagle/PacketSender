#ifndef SUBNETCALC_H
#define SUBNETCALC_H

#include <QDialog>
#include <QNetworkInterface>

namespace Ui
{
class SubnetCalc;
}

class SubnetCalc : public QDialog
{
        Q_OBJECT

    public:
        explicit SubnetCalc(QWidget *parent = 0);
        ~SubnetCalc();

    void populate();
    static QList<QNetworkAddressEntry> nonLoopBackAddresses();
private slots:
        void on_ipEdit_textChanged(const QString &arg1);

        void on_subnetEdit_textChanged(const QString &arg1);

        void on_clearButton_clicked();

        void on_ipsubnetCheckEdit_textChanged(const QString &arg1);

    private:
        Ui::SubnetCalc *ui;
};

#endif // SUBNETCALC_H
