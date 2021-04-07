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

        static const QString ALLHTTPSHOSTS;
        static const QString HTTPHEADERINDEX;

        static QList<int> portsToIntList(QString ports);
        static QString intListToPorts(QList<int> portList);
        void statusBarMessage(QString msg);

        static QStringList getHTTPHeaders(QString host);
        static QHash<QString, QString> getRawHTTPHeaders(QString host);
        static QHash<QString, QStringList> getAllHTTPHeaders();

        static bool detectJSON_XML();
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



        void on_addCredentialButton_clicked();

        void on_httpDeleteHeaderButton_clicked();

        void on_httpCredentialTable_itemChanged(QTableWidgetItem *item);

        void on_genAuthCheck_clicked(bool checked);

private:
        Ui::Settings *ui;
        QList<Packet> packetsSaved;
        QStringList packetTableHeaders;
        QStringList packetSavedTableHeaders;

        bool httpSettingsLoading;

        void setDefaultTableHeaders();
        void setStoredTableHeaders();
        void loadTableHeaders();
        void loadCredentialTable();
        void saveHTTPHeader(QString host, QString header);
        void deleteHTTPHeader(QString host, QString header);
        void clearHTTPHeaders(QString host);
        static QPair<QString, QString> header2keyvalue(QString header);
};

#endif // SETTINGS_H
