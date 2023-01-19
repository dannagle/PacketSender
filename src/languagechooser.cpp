
#include <QPushButton>
#include <QObject>
#include "languagechooser.h"
#include "ui_languagechooser.h"

LanguageChooser::LanguageChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LanguageChooser)
{
    ui->setupUi(this);

    englishClicked();

}

LanguageChooser::~LanguageChooser()
{
    delete ui;
}


void LanguageChooser::englishClicked()
{
    ui->radioEnglish->setChecked(true);
    ui->radioSpanish->setChecked(false);

}

void LanguageChooser::spanishClicked()
{
    ui->radioEnglish->setChecked(false);
    ui->radioSpanish->setChecked(true);

}

void LanguageChooser::on_englishLabel_clicked()
{
    englishClicked();
}


void LanguageChooser::on_spanishLabel_clicked()
{
    spanishClicked();
}

