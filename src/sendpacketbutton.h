/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */
#ifndef SENDPACKETBUTTON_H
#define SENDPACKETBUTTON_H

#include <QPushButton>
#include "globals.h"

class SendPacketButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit SendPacketButton(QWidget *parent = 0);
        QString name;

        void init();
    signals:
        void sendPacket(QString name);

    public slots:
        void sendClicked();

};

#endif // SENDPACKETBUTTON_H
