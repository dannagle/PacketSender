#include "about.h"
#include "globals.h"
#include "ui_about.h"

#include <QDesktopServices>
#include <QDate>
#include <QDebug>
#include <QUrl>
#include <QSslSocket>

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);


    QString versionBuilder = QString("PS Version: ") + SW_VERSION;
    versionBuilder.append("\nQt Version: " + QString(QT_VERSION_STR));
    if (QSslSocket::supportsSsl()) {
        versionBuilder.append("\nSSL Version: ");
        versionBuilder.append(QSslSocket::sslLibraryBuildVersionString());
    }
    ui->buidDateLabel->setText(versionBuilder);

    QIcon mIcon(":pslogo.png");
    setWindowTitle("About Packet Sender");
    setWindowIcon(mIcon);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);



    QPalette pal = ui->psLinkButton->palette();
    pal.setColor(QPalette::Button, QColor(Qt::white));
    ui->psLinkButton->setAutoFillBackground(true);
    ui->psLinkButton->setPalette(pal);


    ui->psLinkButton->setStyleSheet(HYPERLINKSTYLE);
    ui->psLinkButton->setIcon(QIcon(":pslogo.png"));
    ui->psLinkButton->setFlat(true);
    ui->psLinkButton->setCursor(Qt::PointingHandCursor);
    connect(ui->psLinkButton, SIGNAL(clicked()),
            this, SLOT(gotoPacketSenderDotCom()));

}

About::~About()
{
    delete ui;
}

void About::gotoPacketSenderDotCom()
{
    //Open URL in browser
    QDesktopServices::openUrl(QUrl("https://packetsender.com/"));

}

void About::gotoDanNagleDotCom()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("https://dannagle.com/"));

}


void About::gotoDanNaglePayPal()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://dannagle.com/paypal"));

}

void About::gotoNagleCode()
{
    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://twitter.com/NagleCode"));

}
