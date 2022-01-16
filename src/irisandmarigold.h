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
#pragma once
#include <QDialog>

namespace Ui
{
class IrisAndMarigold;
}

class IrisAndMarigold : public QDialog
{
        Q_OBJECT

    public:
        explicit IrisAndMarigold(QWidget *parent = 0);
        ~IrisAndMarigold();

    private:
        Ui::IrisAndMarigold *ui;
};


