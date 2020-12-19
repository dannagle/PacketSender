/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 * (This is the dog easter egg.)
 *
 */


#include "brucethepoodle.h"
#include "ui_brucethepoodle.h"

BruceThePoodle::BruceThePoodle(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BruceThePoodle)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    this->setFixedSize(this->width(), this->height());
    setWindowTitle("Bruce The Poodle!");
}

BruceThePoodle::~BruceThePoodle()
{
    delete ui;
}
