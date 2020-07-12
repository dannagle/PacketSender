#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QList>
#include "globals.h"
#include "packet.h"



namespace Ui
{
class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:
        explicit Settings(QWidget *parent = nullptr);
        ~Settings();

        static QStringList defaultPacketTableHeader();
        static QStringList defaultTrafficTableHeader();

        static const QString SEND_STR;
        static const QString NAME_STR;
        static const QString RESEND_STR;
        static const QString TOADDRESS_STR;
        static const QString TOPORT_STR;
        static const QString METHOD_STR;
        static const QString ASCII_STR;
        static const QString HEX_STR;
        static const QString REQUEST_STR;

        static const QString TIME_STR;
        static const QString FROMIP_STR;
        static const QString FROMPORT_STR;
        static const QString ERROR_STR;

        static QList<int> portsToIntList(QString ports);
        static QString intListToPorts(QList<int> portList);
        void statusBarMessage(QString msg);

private slots:
        void on_buttonBox_accepted();

        void on_asciiResponseEdit_textEdited(const QString &arg1);

        void on_hexResponseEdit_textEdited(const QString &arg1);

        void on_responsePacketBox_currentIndexChanged(const QString &arg1);

        void on_defaultDisplayButton_clicked();

        void on_smartResponseEnableCheck_clicked();

        void on_sslCaPathBrowseButton_clicked();

        void on_sslLocalCertificatePathBrowseButton_clicked();

        void on_sslPrivateKeyPathBrowseButton_clicked();


        void on_documentationButton_clicked();

    private:
        Ui::Settings *ui;
        QList<Packet> packetsSaved;
        QStringList packetTableHeaders;
        QStringList packetSavedTableHeaders;


        void setDefaultTableHeaders();
        void setStoredTableHeaders();
        void loadTableHeaders();


};

#endif // SETTINGS_H
