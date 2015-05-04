#include "about.h"
#include "ui_about.h"

#include <QDesktopServices>
#include <QDate>


About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    QDate vDate = QDate::fromString(QString(__DATE__).simplified(), "MMM d yyyy");
    ui->buidDateLabel->setText("Build date: " + vDate.toString("yyyy-MM-dd"));

    QIcon mIcon(":pslogo.png");
    setWindowTitle("About Packet Sender");
    setWindowIcon(mIcon);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);


    ui->DNLinkButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->DNLinkButton->setFlat(true);
    ui->DNLinkButton->setCursor(Qt::PointingHandCursor);
    connect(ui->DNLinkButton, SIGNAL(clicked()),
            this, SLOT(gotoDanNagleDotCom()));


    ui->twitterButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->twitterButton->setIcon( QIcon(":Twitter_logo_blue.png"));
    ui->twitterButton->setFlat(true);
    ui->twitterButton->setCursor(Qt::PointingHandCursor);
    connect(ui->twitterButton, SIGNAL(clicked()),
            this, SLOT(gotoNagleCode()));


    ui->DNAmazonLinkButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->DNAmazonLinkButton->setIcon( QIcon(":pslogo.png"));
    ui->DNAmazonLinkButton->setFlat(true);
    ui->DNAmazonLinkButton->setCursor(Qt::PointingHandCursor);
    connect(ui->DNAmazonLinkButton, SIGNAL(clicked()),
            this, SLOT(gotoDanNaglePayPal()));


    ui->psLinkButton->setStyleSheet("QPushButton { color: blue; } QPushButton::hover { color: #BC810C; } ");
    ui->psLinkButton->setIcon( QIcon(":pslogo.png"));
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
    QDesktopServices::openUrl(QUrl("http://packetsender.com/"));

}

void About::gotoDanNagleDotCom()
{

    //Open URL in browser
    QDesktopServices::openUrl(QUrl("http://dannagle.com/"));

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
