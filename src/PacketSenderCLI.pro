#-------------------------------------------------
#
# Project created by QtCreator 2012-08-10T12:30:15
#
#-------------------------------------------------

QT       += core network widgets

TARGET = packetsendercli
TEMPLATE = app

win32:CONFIG += console
win32:DEFINES += CONSOLE_BUILD


include(packetsender_main.pri)
