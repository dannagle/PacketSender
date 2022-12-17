#-------------------------------------------------
#
# Project created by QtCreator 2012-08-10T12:30:15
#
#-------------------------------------------------

QT       += core network

TARGET = packetsendercli
TEMPLATE = app

win32:CONFIG += console
win32:DEFINES += CONSOLE_BUILD
win32:DEFINES -= GUI_BUILD


include(packetsender_main.pri)
