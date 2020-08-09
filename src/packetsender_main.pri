	
SOURCES += main.cpp\
    $$PWD/panel.cpp \
        mainwindow.cpp \
    packetnetwork.cpp \
    packet.cpp \
    sendpacketbutton.cpp \
    brucethepoodle.cpp \
    tcpthread.cpp \
    persistentconnection.cpp \
    settings.cpp \
    about.cpp \
    subnetcalc.cpp \
    threadedtcpserver.cpp \
    cloudui.cpp \
	multicastsetup.cpp \
	udpflooding.cpp \
    $$PWD/packetlogmodel.cpp \
        postdatagen.cpp \
    panelgenerator.cpp

win32: SOURCES += persistenthttp.cpp
macx: SOURCES += persistenthttp.cpp


HEADERS  += mainwindow.h \
    $$PWD/panel.h \
    packetnetwork.h \
    packet.h \
    globals.h \
    sendpacketbutton.h \
    brucethepoodle.h \
    tcpthread.h \
    persistentconnection.h \
    settings.h \
    about.h \
    subnetcalc.h \
    threadedtcpserver.h \
    cloudui.h \
	multicastsetup.h \
	udpflooding.h \
    $$PWD/packetlogmodel.h \
        postdatagen.h\
    panelgenerator.h

win32: HEADERS += persistenthttp.h
macx: HEADERS += persistenthttp.h



FORMS    += mainwindow.ui \
    brucethepoodle.ui \
    persistentconnection.ui \
    settings.ui \
    about.ui \
    subnetcalc.ui \
    cloudui.ui \
	multicastsetup.ui \	
    udpflooding.ui \
        persistenthttp.ui \
        postdatagen.ui \
    panelgenerator.ui

OTHER_FILES += \
    packetsender.css \
   packetsender_mac.css

linux:target.path = /usr/local/bin/
linux:INSTALLS += target

linux:install_desktop.path = /usr/share/applications/
linux:install_desktop.files = packetsender.desktop

linux:install_icon.path = /usr/share/icons/
linux:install_icon.files = packetsender.svg

INSTALLS += \
    install_desktop \
    install_icon

# android: include(../../qt_android_openssl/openssl.pri)

win32:RC_FILE = psicon.rc


macx:ICON = psicons.icns

macx:QMAKE_INFO_PLIST = Info.plist

linux:QMAKE_CXXFLAGS += -D_FORTIFY_SOURCE=2

RESOURCES += packetsender.qrc \
    qdarkstyle/style.qrc
