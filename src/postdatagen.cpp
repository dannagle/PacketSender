#include "postdatagen.h"
#include "ui_postdatagen.h"

#include <QtDebug>
#include <QUrlQuery>
#include "globals.h"

/*
POST /formtest.php HTTP/1.1
Host: 127.0.0.1:15000
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.14; rv:67.0) Gecko/20100101 Firefox/67.0
Accept: text/html
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
Referer: http://127.0.0.1:8000/formtest.php
Content-Type: application/x-www-form-urlencoded
Content-Length: 24
Connection: keep-alive
Upgrade-Insecure-Requests: 1

param1=boom&param2=boom2Server
*/

PostDataGen::PostDataGen(QWidget *parent, QString query) :
    QDialog(parent), ui(new Ui::PostDataGen)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle("Post Data Generator");

    QUrlQuery qUrl = QUrlQuery(query);

    paramList = qUrl.queryItems();


    generateParamUI();

}

void PostDataGen::generateParamUI() {

    static bool suppress = false;

    if(suppress) return;

    suppress = true;

    while(ui->gridLayout->count() > 0) {

        QLayoutItem * w = ui->gridLayout->takeAt(0);
        if(w->widget()) {
            delete w->widget();
        }
        if(w->layout()) {
            delete w->layout();
        }
    }


    QPair<QString,QString> key;

    int row = 0;
    foreach(key, paramList) {

        QPushButton * deleteButton = new QPushButton("X");

        QLineEdit * keyLabel = new QLineEdit(key.first);
        QLineEdit * valLabel = new QLineEdit(key.second);

        deleteButton->setProperty("key", key.first);
        deleteButton->setProperty("val", key.second);
        deleteButton->setProperty("row", row);

        keyLabel->setProperty("key", key.first);
        keyLabel->setProperty("val", key.second);
        keyLabel->setProperty("row", row);
        keyLabel->setProperty("iskey", true);

        valLabel->setProperty("key", key.first);
        valLabel->setProperty("val", key.second);
        valLabel->setProperty("row", row);
        valLabel->setProperty("iskey", false);


        if (!connect(deleteButton, &QPushButton::clicked, this, &PostDataGen::removeParam)) {
            QDEBUG() << key << "connection false";
        }

        if (!connect(keyLabel, &QLineEdit::editingFinished, this, &PostDataGen::editParam)) {
            QDEBUG() << key << "connection false";
        }

        if (!connect(valLabel, &QLineEdit::editingFinished, this, &PostDataGen::editParam)) {
            QDEBUG() << key << "connection false";
        }


        ui->gridLayout->addWidget(deleteButton, row, 0);
        ui->gridLayout->addWidget(keyLabel, row, 1);
        ui->gridLayout->addWidget(valLabel, row, 2);

        row++;

    }

    suppress = false;



}


PostDataGen::~PostDataGen()
{
    delete ui;
}

void PostDataGen::on_buttonBox_accepted()
{
    QString data = "Boom worked";

    QUrlQuery params;


    QPair<QString,QString> key;
    foreach(key, paramList) {
        params.addQueryItem(key.first, key.second);
    }

    QDEBUGVAR(params.query());

    emit postGenerated(params.query());
}

void PostDataGen::on_addParamButton_clicked()
{
    QString param = ui->paramNameEdit->text();
    QString val = ui->paramVlueEdit->text();

    QPair<QString,QString> key;
    if(!param.isEmpty()) {
        key.first = param;
        key.second = val;
        paramList.append(key);
        generateParamUI();
    }

    QDEBUG();
}


void PostDataGen::editParam()
{
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
    if( edit != nullptr ) {

        QString newval = edit->text();
        bool iskey  = edit->property("iskey").toBool();
        int row = edit->property("row").toInt();

        if(row < paramList.size()) {
            if(iskey) {
                paramList[row].first = newval;
            } else {
                paramList[row].second = newval;
            }
        }

        //generateParamUI();
    }
}

void PostDataGen::removeParam()
{

    // e.g. casting to the class you know its connected with
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if( button != nullptr ) {

        int row = button->property("row").toInt();
        if(row < paramList.size()) {
            paramList.removeAt(row);
        }
        generateParamUI();
    }

}
