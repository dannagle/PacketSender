#include "languagechooser.h"
#include "ui_languagechooser.h"

LanguageChooser::LanguageChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LanguageChooser)
{
    ui->setupUi(this);
}

LanguageChooser::~LanguageChooser()
{
    delete ui;
}
