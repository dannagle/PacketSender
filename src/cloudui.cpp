#include "cloudui.h"
#include "ui_cloudui.h"

#include <QList>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMessageBox>


#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "globals.h"

CloudUI::CloudUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CloudUI)
{
    ui->setupUi(this);


    packetsToImport.clear();
    packetsToExport.clear();
    packetsFound.clear();
    packetSets.clear();

    ui->cloudTabWidget->setCurrentIndex(0);



    http = new QNetworkAccessManager(this); //Cloud UI http object

    if(! connect(http, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(replyFinished(QNetworkReply*))) ) {

        QDEBUG() << "http request finished connection false";

    }


    QIcon mIcon(":pslogo.png");
    setWindowTitle("Packet Sender Cloud");
    setWindowIcon(mIcon);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);


}

CloudUI::~CloudUI()
{
    delete ui;
}



void CloudUI::replyFinished(QNetworkReply* request)
{
    QByteArray data = request->readAll();
    QDEBUG() <<"Request complete:" << data.size();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    ui->loginButton->setEnabled(true);


    bool success = false;

    if(!doc.isNull()) {
        //valid json
        if(doc.isArray()) {
            //valid array
            QJsonArray jsonArray = doc.array();
            if(!jsonArray.isEmpty()) {
                QDEBUG() << "Found" <<  jsonArray.size() << "sets";
                packetSets.clear();

                for(int i=0; i < jsonArray.size(); i++) {
                    PacketSet pktSet;
                    QJsonObject json = jsonArray[i].toObject();
                    pktSet.description = json["description"].toString();
                    QByteArray jsonData = json["packetjson"].toString().toLatin1();
                    if(!jsonData.isEmpty()) {
                        pktSet.packets = Packet::ImportJSON(jsonData);
                        QDEBUG() << "Set" << pktSet.description << "has" << pktSet.packets.size() << "packets";

                        if(pktSet.packets.size() > 0) {
                            success = true;
                            packetSets.append(pktSet);
                        }
                    } else {
                        QDEBUG() << "Packet array is null";
                    }

                }

            }
        }
    }

    if(success) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Success");
        msgBox.setStandardButtons(QMessageBox::Ok );
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Found " + QString::number(packetSets.size()) + " sets of packets!");
        msgBox.exec();
        loadPacketSetTable();
        ui->cloudTabWidget->setCurrentIndex(1);


    } else {

        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setStandardButtons(QMessageBox::Ok );
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Did not fetch any packets");
        msgBox.exec();

    }


}


void CloudUI::loadPacketSetTable()
{
    ui->packetSetTable->clear();

    QStringList tableHeaders;
    tableHeaders << "Description" << "Packet Count";


    ui->packetSetTable->setColumnCount(2);

    ui->packetSetTable->verticalHeader()->show();
    ui->packetSetTable->horizontalHeader()->show();
    ui->packetSetTable->setHorizontalHeaderLabels(tableHeaders);


    ui->packetSetTable->setRowCount(packetSets.size());
    for(int i=0; i < packetSets.size(); i++ ) {
        QTableWidgetItem  * itemDescription = new QTableWidgetItem(packetSets[i].description);
        QTableWidgetItem  * itemSize = new QTableWidgetItem(QString::number(packetSets[i].packets.size()));

        itemDescription->setData(Qt::UserRole, i); //set index
        itemSize->setData(Qt::UserRole, i); //set index

        ui->packetSetTable->setItem(i, 0, itemDescription);
        ui->packetSetTable->setItem(i, 1, itemSize);
    }


    ui->packetSetTable->setSortingEnabled(true);

    ui->packetSetTable->resizeColumnsToContents();
    ui->packetSetTable->resizeRowsToContents();
    ui->packetSetTable->horizontalHeader()->setStretchLastSection( true );


}



void CloudUI::on_loginButton_clicked()
{

    QString un = ui->usernameEdit->text().trimmed();
    QString pw = ui->passwordEdit->text();

    ui->loginButton->setEnabled(false);

    QString requestURL = CLOUD_URL + QString("?un=") + un + "&pw=" + pw;

    QDEBUGVAR(requestURL);

    http->get(QNetworkRequest(QUrl(requestURL)));

}

void CloudUI::on_exportButton_clicked()
{
    QDEBUG();

}

void CloudUI::on_viewPublicButton_clicked()
{
    QDEBUG();

}

void CloudUI::on_importURLButton_clicked()
{
    QDEBUG();

}

void CloudUI::on_importPacketsButton_clicked()
{
    QDEBUG();

}


