
#include <QPushButton>
#include <QObject>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include "languagechooser.h"
#include "settings.h"
#include "globals.h"
#include "ui_languagechooser.h"

LanguageChooser::LanguageChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LanguageChooser)
{
    ui->setupUi(this);


    QString language = Settings::language().toLower();

    QDEBUGVAR(language);

    on_englishLabel_clicked();

    if(language.contains("spanish")) {
        on_spanishLabel_clicked();
    }

    if(language.contains("german")) {
        on_germanLabel_clicked();
    }


}

LanguageChooser::~LanguageChooser()
{
    delete ui;
}


void LanguageChooser::preClicked()
{
    ui->radioSpanish->setChecked(false);
    ui->radioEnglish->setChecked(true);
    ui->radioGerman->setChecked(true);

}

void LanguageChooser::on_englishLabel_clicked()
{
    preClicked();
    ui->radioEnglish->setChecked(true);
    ui->englishLabel->setFocus();
}


void LanguageChooser::on_spanishLabel_clicked()
{
    preClicked();
    ui->radioSpanish->setChecked(true);
    ui->spanishLabel->setFocus();
}


void LanguageChooser::on_germanLabel_clicked()
{
    preClicked();
    ui->radioGerman->setChecked(true);
    ui->germanLabel->setFocus();
}


void LanguageChooser::on_okButton_clicked()
{
    QSettings settings(SETTINGSFILE, QSettings::IniFormat);


    // Default is English
    settings.setValue("languageCombo", "English");


    if(ui->radioSpanish->isChecked()) {
        settings.setValue("languageCombo", "Spanish");
    }

    if(ui->radioGerman->isChecked()) {
        settings.setValue("languageCombo", "German");
    }


    this->close();

}

