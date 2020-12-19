
#ifdef CHROMIUM

#include "persistenthttp.h"
#include "ui_persistenthttp.h"

#include <QFontDatabase>
#include <QClipboard>
#include <QWebEngineSettings>

PersistentHTTP::PersistentHTTP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersistentHTTP)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
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

    setWindowTitle("Code/Render "+url.toString());

    ui->webView->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    ui->webView->setHtml(QString(thedata));
    ui->webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->copyRenderButton->hide(); // for now

}

PersistentHTTP::~PersistentHTTP()
{
    /*
    ui->webView->close();
    ui->webView->page()->deleteLater();
    */
    delete ui;
}

void PersistentHTTP::on_copyCodeButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString(data));


}

void PersistentHTTP::on_copyRenderButton_clicked()
{
    ui->webView->setEnabled(true);
    ui->webView->setFocus();
    ui->webView->triggerPageAction(QWebEnginePage::SelectAll);
    ui->webView->triggerPageAction(QWebEnginePage::Copy);
    //ui->webView->triggerPageAction(QWebEnginePage::Unselect);


}
#endif

