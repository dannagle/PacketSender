
SOURCES += main.cpp\
        mainpacketreceiver.cpp \
    packetnetwork.cpp \
    packet.cpp \
    settings.cpp \
    tcpthread.cpp \
    threadedtcpserver.cpp


HEADERS  += mainpacketreceiver.h \
    packetnetwork.h \
    packet.h \
    globals.h \
    settings.h \
    tcpthread.h \
    threadedtcpserver.h



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

# Enable before porting to Qt6
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""


macx:ICON = psicons.icns

macx:QMAKE_INFO_PLIST = Info.plist

linux:QMAKE_CXXFLAGS += -D_FORTIFY_SOURCE=2

RESOURCES += packetsender.qrc \
    $$PWD/translations.qrc \
    qdarkstyle/style.qrc
