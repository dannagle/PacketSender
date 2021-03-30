#  Project file used by snapcraft

QT  += core gui network widgets

DEFINES += "ISSNAP=1"


TARGET = packetsender
TEMPLATE = app

include(packetsender_main.pri)
