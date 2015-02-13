/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */

#include <QDebug>
#include "sendpacketbutton.h"


SendPacketButton::SendPacketButton(QWidget *parent) :
    QPushButton(parent)
{

}

void SendPacketButton::init()
{
    //QDEBUG() << " sendButton connect attempt:" <<
    connect(this, SIGNAL(clicked()), this, SLOT(sendClicked()));
}


void SendPacketButton::sendClicked()
{

    QDEBUG() << " Emit clicked: " << name;

    emit sendPacket(name);

}
