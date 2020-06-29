#ifndef CLOUDUI_H
#define CLOUDUI_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QList>
#include <QStringList>

#include "packet.h"


namespace Ui
{
class CloudUI;
}

typedef struct PacketSet {
    QString name;
    QString description;
    QString lastupdate;
    QString path;
    int ispublic;
    QList<Packet> packets;
} PacketSet;



class CloudUI : public QDialog
{
        Q_OBJECT

    public:
        explicit CloudUI(QWidget *parent = 0);
        ~CloudUI();

    signals:
        void packetsImported(QList<Packet> packetSet);


    private slots:
        void on_loginButton_clicked();

        void on_saveToCloudButton_clicked();

        void on_viewPublicButton_clicked();

        void on_importURLButton_clicked();

        void on_importPacketsButton_clicked();

        void replyFinished(QNetworkReply *request);

        void on_createAccountButton_clicked();

        void on_privacyButton_clicked();

        void on_termsButton_clicked();


        void on_deletePacketButton_clicked();

        void on_cloudLinkButton_clicked();

        void on_packetSetTable_clicked(const QModelIndex &index);

    private:
        void loadPacketSetTable();

        void popMsg(QString title, QString msg, bool isError);

        void doPost(QUrlQuery postData);

        bool suppressAlert;


        QString un;
        QString pw;

        QList<Packet> packets;

        Ui::CloudUI *ui;
        QList<Packet> packetsToImport;
        QList<Packet> packetsToExport;
        QList<Packet> packetsFound;
        QList<PacketSet> packetSets;


        QNetworkAccessManager  * http;


};

#endif // CLOUDUI_H
