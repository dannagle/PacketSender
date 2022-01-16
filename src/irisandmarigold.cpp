/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 * (This is the puppy easter egg.)
 *
 */


#include "irisandmarigold.h"
#include "ui_irisandmarigold.h"

IrisAndMarigold::IrisAndMarigold(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IrisAndMarigold)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    this->setFixedSize(this->width(), this->height());
    setWindowTitle("Iris And Marigold");
}

IrisAndMarigold::~IrisAndMarigold()
{
    delete ui;
}
