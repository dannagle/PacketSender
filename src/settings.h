#ifndef SETTINGS_H
#define SETTINGS_H

#include "globals.h"
#include "mainwindow.h"

#ifndef CONSOLE_BUILD
#include <QDialog>
#include "packet.h"
#endif
#include <QSettings>
#include <QList>
#include <QCheckBox>


#ifdef CONSOLE_BUILD
class Settings {
public:
    static QList<int> portsToIntList(QString ports);
    static QString intListToPorts(QList<int> portList);

    static QStringList getHTTPHeaders(QString host);
    static QHash<QString, QString> getRawHTTPHeaders(QString host);
    static QHash<QString, QStringList> getAllHTTPHeaders();

    static bool detectJSON_XML();


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
    static QString logHeaderTranslate(QString txt);


private:
    static QPair<QString, QString> header2keyvalue(QString header);
    void on_chooseLanguageButton_clicked();



};
#else


namespace Ui
{
class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:
        MainWindow* rmw;
        QCheckBox* sendSimpleAck;
        QCheckBox* sendSmartResponse;
        QLineEdit* sslLocalCertificatePath;
        QString initialSslLocalCertificatePath;
        QString initialSslCaPath;
        QString initialSslPrivateKeyPath;

        explicit Settings(QWidget *parent = nullptr, MainWindow* mw = nullptr);
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
        static QString language();
        static bool needLanguage();
        static QString logHeaderTranslate(QString txt);

        static bool useDark();
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

        void on_chooseLanguageButton_clicked();

private:
        QWidget *parentWidget;
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
    signals:
        void loadingCertsAgain();
};

#endif
#endif // SETTINGS_H
