#include "cloudui.h"
#include "ui_cloudui.h"

#include <QList>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QModelIndexList>
#include <QModelIndex>
#include <QSettings>
#include <QUrlQuery>
#include <QDesktopServices>

bool isLetterNumUnder(QString str);

void popMsg(QString title, QString msg, bool isError);

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "globals.h"


void linkifyButton(QPushButton * btn)
{

    QPalette pal = btn->palette();
    pal.setColor(QPalette::Button, QColor(Qt::white));
    btn->setAutoFillBackground(true);
    btn->setPalette(pal);


    btn->setStyleSheet(HYPERLINKSTYLE);
    btn->setFlat(true);
    btn->setCursor(Qt::PointingHandCursor);
}

CloudUI::CloudUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CloudUI)
{
    ui->setupUi(this);


    packetsToImport.clear();
    packetsToExport.clear();
    packetsFound.clear();
    packetSets.clear();

    ui->passwordConfirmEdit->hide();
    ui->passwordConfirmLabel->hide();

    ui->viewPublicButton->hide();


    ui->cloudTabWidget->setCurrentIndex(0);

    ui->createAccountButton->setIcon(QIcon("://icons/ic_person_black_24dp_2x.png"));
    linkifyButton(ui->createAccountButton);

    linkifyButton(ui->termsButton);
    linkifyButton(ui->privacyButton);
    linkifyButton(ui->cloudLinkButton);

    suppressAlert = false;


    http = new QNetworkAccessManager(this); //Cloud UI http object

    if (! connect(http, SIGNAL(finished(QNetworkReply*)),
                  this, SLOT(replyFinished(QNetworkReply*)))) {

        QDEBUG() << "http request finished connection false";

    }


    QIcon mIcon("://icons/ic_cloud_done_black_24dp_2x.png");
    setWindowTitle("Packet Sender Cloud");
    setWindowIcon(mIcon);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);


    QSettings settings(SETTINGSFILE, QSettings::IniFormat);

    ui->usernameEdit->setText(settings.value("cloudUsername", "").toString());
    ui->passwordEdit->setText(settings.value("cloudPassword", "").toString());
    ui->rememberLoginCheck->setChecked(settings.value("rememberLoginCheck", false).toBool());

    ui->usernameEdit->setFocus();


    packets = Packet::fetchAllfromDB("");

    ui->shareBlurbLabel->setText("Saving " + QString::number(packets.size()) + " packet set to cloud");


    settings.setValue("cloudPassword", ui->passwordEdit->text());

}

CloudUI::~CloudUI()
{
    delete ui;
}

QString getpw64(QString pw)
{
    return QString(pw.toLatin1().toBase64());
}

void CloudUI::popMsg(QString title, QString msg, bool isError)
{

    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setWindowIcon(QIcon("://icons/ic_cloud_done_black_24dp_2x.png"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    if (isError) {
        msgBox.setIcon(QMessageBox::Warning);
    } else {
        msgBox.setIcon(QMessageBox::Information);

    }
    msgBox.setText(msg);

    if (!suppressAlert) {
        msgBox.exec();
    }

    suppressAlert = false;



}

bool isLetterNumUnder(QString str)
{

    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '_') {
            continue;
        }
        if (str[i].isDigit()) {
            continue;
        }
        if (str[i].isLetter())  {
            continue;
        }

        return false;
    }

    return true;
}


void CloudUI::replyFinished(QNetworkReply* request)
{
    QByteArray data = request->readAll();
    QString dataString = QString(data).trimmed();

    QString dataStringDebug = dataString;
    dataStringDebug.truncate(200);
    QDEBUG() << "Request complete:" << dataStringDebug;

    ui->loginButton->setEnabled(true);
    ui->importURLButton->setEnabled(true);

    if (dataString.toLower().startsWith("success")) {
        popMsg("Success", dataString, false);
        if (ui->passwordConfirmEdit->isVisible()) {
            ui->cloudTabWidget->setCurrentIndex(2);
            on_createAccountButton_clicked();
        }

        suppressAlert = true;
        on_loginButton_clicked();

        return;
    }

    if (dataString.toLower().startsWith("error")) {
        popMsg("Error", dataString, true);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);



    bool success = false;

    if (!doc.isNull()) {
        //valid json
        if (doc.isArray()) {
            //valid array
            QJsonArray jsonArray = doc.array();
            if (!jsonArray.isEmpty()) {
                QDEBUG() << "Found" <<  jsonArray.size() << "sets";
                packetSets.clear();

                for (int i = 0; i < jsonArray.size(); i++) {
                    PacketSet pktSet;
                    QJsonObject json = jsonArray[i].toObject();
                    pktSet.name  = json["name"].toString();
                    pktSet.lastupdate  = json["lastupdate"].toString();
                    pktSet.description = json["description"].toString();
                    pktSet.path = json["path"].toString();
                    pktSet.ispublic = json["public"].toString().toInt();
                    QByteArray jsonData = json["packetjson"].toString().toUtf8();
                    if (!jsonData.isEmpty()) {
                        pktSet.packets = Packet::ImportJSON(jsonData);
                        QDEBUG() << "Set" << pktSet.name << "has" << pktSet.packets.size() << "packets";

                        if (pktSet.packets.size() > 0) {
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

    if (success) {
        //popMsg("Success", "Found " + QString::number(packetSets.size()) + " sets of packets!", false);
        loadPacketSetTable();
        ui->cloudTabWidget->setCurrentIndex(1);


    } else {
        popMsg("Error", "Did not fetch any packets", true);

    }


}


void setuptWidget(QTableWidgetItem  * item, QList<PacketSet> packetSets, int i)
{
    item->setData(Qt::UserRole, i); //set index
    item->setData(Qt::UserRole + 1, packetSets[i].path); //set path
    item->setData(Qt::UserRole + 2, packetSets[i].ispublic); //set public
    item->setToolTip(packetSets[i].description);

}
void CloudUI::loadPacketSetTable()
{
    ui->packetSetTable->clear();
    ui->viewPublicButton->hide();

    QStringList tableHeaders;
    tableHeaders << "Name" << "Packet Count" << "Uploaded" << "Path" << "Public";


    ui->packetSetTable->setColumnCount(tableHeaders.size());

    ui->packetSetTable->verticalHeader()->show();
    ui->packetSetTable->horizontalHeader()->show();
    ui->packetSetTable->setHorizontalHeaderLabels(tableHeaders);


    ui->packetSetTable->setRowCount(packetSets.size());
    for (int i = 0; i < packetSets.size(); i++) {
        QTableWidgetItem  * itemName = new QTableWidgetItem(packetSets[i].name);
        QTableWidgetItem  * itemSize = new QTableWidgetItem(QString::number(packetSets[i].packets.size()));
        QTableWidgetItem  * lastupdate = new QTableWidgetItem(packetSets[i].lastupdate);

        QTableWidgetItem  * itemPath = new QTableWidgetItem(packetSets[i].path);
        QTableWidgetItem  * itemPublic = new QTableWidgetItem(QString::number(packetSets[i].ispublic));


        setuptWidget(itemName, packetSets, i);
        setuptWidget(itemSize, packetSets, i);
        setuptWidget(lastupdate, packetSets, i);
        setuptWidget(itemPath, packetSets, i);
        setuptWidget(itemPublic, packetSets, i);


        ui->packetSetTable->setItem(i, 0, itemName);
        ui->packetSetTable->setItem(i, 1, itemSize);
        ui->packetSetTable->setItem(i, 2, lastupdate);
        ui->packetSetTable->setItem(i, 3, itemPath);
        ui->packetSetTable->setItem(i, 4, itemPublic);
    }


    ui->packetSetTable->setSortingEnabled(true);

    ui->packetSetTable->resizeColumnsToContents();
    ui->packetSetTable->resizeRowsToContents();
    ui->packetSetTable->horizontalHeader()->setStretchLastSection(true);


}



void CloudUI::on_loginButton_clicked()
{

    QUrlQuery postData;

    QString un = ui->usernameEdit->text().trimmed();

    if (!isLetterNumUnder(un)) {
        popMsg("Invalid.", "Usernames may only be letters, numbers, underscores", true);
        ui->usernameEdit->setFocus();
        return;
    }

    if (un.size() < 3) {
        popMsg("Too short.", "Username must be at least 3 characters.", true);
        ui->usernameEdit->setFocus();
        return;
    }

    QString pw = ui->passwordEdit->text().trimmed();
    if (pw.size() < 3) {
        popMsg("Too short.", "Passwords must be at least 3 characters.", true);
        ui->usernameEdit->setFocus();
        return;
    }


    QString pw64 = getpw64(ui->passwordEdit->text());
    postData.addQueryItem("un", ui->usernameEdit->text());
    postData.addQueryItem("pw64", pw64);

    if (ui->passwordConfirmEdit->isVisible()) {
        if (ui->passwordConfirmEdit->text() != ui->passwordEdit->text()) {
            popMsg("Mismatch.", "Passwords do not match.", true);
            ui->passwordEdit->setFocus();
            return;
        }
        postData.addQueryItem("newaccount", "1");
    }


    ui->loginButton->setEnabled(false);


    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    settings.setValue("rememberLoginCheck", ui->rememberLoginCheck->isChecked());


    if (ui->rememberLoginCheck->isChecked()) {
        settings.setValue("cloudUsername", ui->usernameEdit->text());
        settings.setValue("cloudPassword", ui->passwordEdit->text());
    }

    doPost(postData);

}


void CloudUI::doPost(QUrlQuery postData)
{

    postData.addQueryItem("swver", SW_VERSION);
    postData.addQueryItem("desktop", "1");
    QNetworkRequest request(QUrl(CLOUD_URL));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    QByteArray pData = postData.toString(QUrl::FullyEncoded).toUtf8();
    QDEBUG() << (CLOUD_URL); // << QString(pData);
    http->post(request, pData);

}

void CloudUI::on_saveToCloudButton_clicked()
{
    QDEBUG();
    QUrlQuery postData;
    QString pw64 = getpw64(ui->passwordEdit->text());
    postData.addQueryItem("un", ui->usernameEdit->text());
    postData.addQueryItem("pw64", pw64);
    QString packets64 = QString(Packet::ExportJSON(packets).toBase64());
    postData.addQueryItem("packets64", packets64);
    postData.addQueryItem("count", QString::number(packets.size()));
    postData.addQueryItem("path", ui->publicPathEdit->text());

    QString pname = ui->packetSetNameEdit->text().trimmed();

    if (pname.isEmpty()) {
        popMsg("Empty", "Set name cannot be empty", true);
        return;
    }

    postData.addQueryItem("setname", pname);
    QString pubblurb = ui->descriptionExportEdit->toPlainText().trimmed();
    postData.addQueryItem("pubblurb", pubblurb);

    if (ui->makePublicCheck->isChecked()) {
        postData.addQueryItem("makepublic", "1");
        if (pubblurb.isEmpty()) {
            popMsg("Empty", "Public description cannot be empty", true);
            return;
        }
    }

    doPost(postData);


}

void CloudUI::on_viewPublicButton_clicked()
{
    QDesktopServices::openUrl(QUrl(ui->viewPublicButton->text()));


}

void CloudUI::on_importURLButton_clicked()
{
    QString url = ui->importURLEdit->text().trimmed();

    if (!url.endsWith("/json")) {
        url.append("/json");
    }

    QDEBUGVAR(url);
    http->get(QNetworkRequest(QUrl(url)));

}

void CloudUI::on_importPacketsButton_clicked()
{

    QModelIndexList indexes = ui->packetSetTable->selectionModel()->selectedIndexes();
    QDEBUGVAR(indexes.size());

    if (indexes.size() > 0) {
        QModelIndex index = indexes.first();
        int packetsetindex = index.data(Qt::UserRole).toInt();
        QDEBUGVAR(packetsetindex);
        if (packetSets.size() > packetsetindex) {
            emit packetsImported(packetSets[packetsetindex].packets);
        }
    }


}




void CloudUI::on_createAccountButton_clicked()
{
    if (ui->passwordConfirmEdit->isVisible()) {
        ui->passwordConfirmEdit->hide();
        ui->passwordConfirmLabel->hide();
        ui->createAccountButton->setText("Create a new account.");
        ui->loginButton->setText("Login");
    } else {
        ui->passwordConfirmEdit->show();
        ui->passwordConfirmLabel->show();
        ui->createAccountButton->setText("Login instead.");
        ui->loginButton->setText("Sign-up");
    }

}

void CloudUI::on_privacyButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://cloud.packetsender.com/privacy"));
}

void CloudUI::on_termsButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://cloud.packetsender.com/termsofuse"));

}

void CloudUI::on_cloudLinkButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://cloud.packetsender.com/help"));

}


void CloudUI::on_deletePacketButton_clicked()
{

    QUrlQuery postData;
    QString un = ui->usernameEdit->text().trimmed();
    QString pw64 = getpw64(ui->passwordEdit->text());
    postData.addQueryItem("un", ui->usernameEdit->text());
    postData.addQueryItem("pw64", pw64);

    QModelIndexList indexes = ui->packetSetTable->selectionModel()->selectedIndexes();
    QDEBUGVAR(indexes.size());

    QString name = "";
    int packetsetindex = -1;

    if (indexes.size() > 0) {
        QModelIndex index = indexes.first();
        packetsetindex = index.data(Qt::UserRole).toInt();
        if (packetSets.size() > packetsetindex) {
            name = packetSets[packetsetindex].name;
        }
    } else {
        popMsg("Empty", "Please select a set.", true);
        return;
    }

    if (name.isEmpty() || packetsetindex < 0) {
        popMsg("Not found", "Set was not found.", true);
        return;
    }

    postData.addQueryItem("deleteset", name);

    QMessageBox msgBox;
    msgBox.setWindowTitle("Delete Set");
    msgBox.setWindowIcon(QIcon("://icons/ic_cloud_done_black_24dp_2x.png"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Delete the set " + name + " from cloud?");
    int yesno = msgBox.exec();
    if (yesno == QMessageBox::Yes) {
        packetSets.removeAt(packetsetindex);
        loadPacketSetTable();

        doPost(postData);
    }

}

void CloudUI::on_packetSetTable_clicked(const QModelIndex &index)
{
    QString path = index.data(Qt::UserRole + 1).toString();
    int ispublic  = index.data(Qt::UserRole + 2).toInt();
    ui->viewPublicButton->setText(path);
    if (ispublic) {
        ui->viewPublicButton->show();
    } else {
        ui->viewPublicButton->hide();
    }
}
