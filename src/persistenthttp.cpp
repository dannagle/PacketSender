
#include "persistenthttp.h"
#include "ui_persistenthttp.h"

#include "globals.h"

#include <QFontDatabase>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>


PersistentHTTP::PersistentHTTP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersistentHTTP)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    tempFiles.clear();
}

void PersistentHTTP::init(QByteArray thedata, QUrl url)
{
    data = thedata;
    ui->codeView->setPlainText(QString(thedata));
    ui->codeView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    int linecount = data.count('\n');
    if (linecount > 5) {
        ui->codeView->setWordWrapMode(QTextOption::NoWrap);
    }
    ui->codeView->setReadOnly(true);

    setWindowTitle("HTTP "+url.toString());


    // holds temporary files until window closes
    QString dl  = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if(!QFile(dl).exists()) {
        dl = QDir::homePath();
    }


    QDateTime now = QDateTime::currentDateTime();
    QString nowString = now.toString("yyyy-MM-dd_hh_mm_ss");
    ui->browserViewButton->setProperty("html-download", dl + "/" +nowString + "-packetsender-httpview.html");
    ui->browserViewButton->setText(nowString + "-packetsender.html");

}

PersistentHTTP::~PersistentHTTP()
{
    QString f;
    foreach(f, tempFiles) {
        QFile(f).remove();
    }

    delete ui;
}

void PersistentHTTP::on_copyCodeButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString(data));


}


void PersistentHTTP::on_browserViewButton_clicked()
{

    QString dl = ui->browserViewButton->property("html-download").toString();
    QFile file(dl);
    if (file.open(QFile::WriteOnly)) {
        file.write(data);
        file.close();

        QFileInfo fileInfo(file);

        QDEBUGVAR(fileInfo.canonicalFilePath());
        tempFiles.append(fileInfo.canonicalFilePath());

        //tempReferences.append(file);

        QDesktopServices::openUrl(QUrl(fileInfo.canonicalFilePath()));

    }

}
