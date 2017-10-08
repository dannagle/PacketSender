#include "cloudui.h"
#include "ui_cloudui.h"

CloudUI::CloudUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CloudUI)
{
    ui->setupUi(this);
}

CloudUI::~CloudUI()
{
    delete ui;
}
