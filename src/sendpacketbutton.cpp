/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */

#include <QDebug>
#include "sendpacketbutton.h"

void themeTheButton(QPushButton * button);

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


//used by mainwindow.cpp and panelgenerator.cpp
void themeTheButton(QPushButton * button)
{
    button->setCursor(Qt::PointingHandCursor);
    button->update();
    return;
    QPalette pal = button->palette();
    pal.setColor(QPalette::Button, QColor("#F5F5F5"));
    button->setAutoFillBackground(true);
    button->setPalette(pal);
    button->setStyleSheet("QPushButton { color: white; background-color: #505F69; } QPushButton::hover { color: #BC810C; background-color: #505F69; } ");

}
