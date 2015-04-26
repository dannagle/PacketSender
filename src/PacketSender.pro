#-------------------------------------------------
#
# Project created by QtCreator 2012-08-10T12:30:15
#
#-------------------------------------------------

QT       += core gui network widgets

TARGET = PacketSender
TEMPLATE = app

#enable only if compiling .com for windows
#win32:CONFIG += console
#win32:DEFINES += CONSOLE_BUILD

SOURCES += main.cpp\
        mainwindow.cpp \
    packetnetwork.cpp \
    packet.cpp \
    sendpacketbutton.cpp \
    brucethepoodle.cpp \
    tcpthread.cpp \
    persistentconnection.cpp

HEADERS  += mainwindow.h \
    packetnetwork.h \
    packet.h \
    globals.h \
    sendpacketbutton.h \
    brucethepoodle.h \
    tcpthread.h \
    persistentconnection.h

FORMS    += mainwindow.ui \
    brucethepoodle.ui \
    persistentconnection.ui


macx:CONFIG += app_bundle


OTHER_FILES += \
    packetsender.css \
   packetsender_mac.css


win32:RC_FILE = psicon.rc


macx:ICON = psicons.icns



RESOURCES += packetsender.qrc
