	
SOURCES += main.cpp\
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
    $$PWD/packetlogmodel.cpp

HEADERS  += mainwindow.h \
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
    ebcdic_ascii_map.h \
    threadedtcpserver.h \
    cloudui.h \
	multicastsetup.h \
	udpflooding.h \
    $$PWD/packetlogmodel.h

FORMS    += mainwindow.ui \
    brucethepoodle.ui \
    persistentconnection.ui \
    settings.ui \
    about.ui \
    subnetcalc.ui \
    cloudui.ui \
	multicastsetup.ui \	
    udpflooding.ui

OTHER_FILES += \
    packetsender.css \
   packetsender_mac.css

linux:target.path = /usr/local/bin/
linux:INSTALLS += target


win32:RC_FILE = psicon.rc


macx:ICON = psicons.icns

macx:QMAKE_INFO_PLIST = Info.plist


RESOURCES += packetsender.qrc
