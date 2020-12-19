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
#ifndef BRUCETHEPOODLE_H
#define BRUCETHEPOODLE_H

#include <QDialog>

namespace Ui
{
class BruceThePoodle;
}

class BruceThePoodle : public QDialog
{
        Q_OBJECT

    public:
        explicit BruceThePoodle(QWidget *parent = 0);
        ~BruceThePoodle();

    private:
        Ui::BruceThePoodle *ui;
};

#endif // BRUCETHEPOODLE_H
