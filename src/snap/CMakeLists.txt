cmake_minimum_required(VERSION 3.16)
project(packetsender VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(SRCROOT /root/project/src)

include_directories(${SRCROOT})

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network Widgets)

set(SOURCES
    ${SRCROOT}/about.cpp
    ${SRCROOT}/brucethepoodle.cpp
    ${SRCROOT}/cloudui.cpp
    ${SRCROOT}/irisandmarigold.cpp
    ${SRCROOT}/languagechooser.cpp
    ${SRCROOT}/main.cpp
    ${SRCROOT}/mainpacketreceiver.cpp
    ${SRCROOT}/mainwindow.cpp
    ${SRCROOT}/multicastsetup.cpp
    ${SRCROOT}/packet.cpp
    ${SRCROOT}/packetlogmodel.cpp
    ${SRCROOT}/packetnetwork.cpp
    ${SRCROOT}/panel.cpp
    ${SRCROOT}/panelgenerator.cpp
    ${SRCROOT}/persistentconnection.cpp
    ${SRCROOT}/persistenthttp.cpp
    ${SRCROOT}/postdatagen.cpp
    ${SRCROOT}/sendpacketbutton.cpp
    ${SRCROOT}/settings.cpp
    ${SRCROOT}/subnetcalc.cpp
    ${SRCROOT}/tcpthread.cpp
    ${SRCROOT}/threadedtcpserver.cpp
    ${SRCROOT}/udpflooding.cpp
    ${SRCROOT}/wakeonlan.cpp
)

set(HEADERS 
    ${SRCROOT}/about.h
    ${SRCROOT}/brucethepoodle.h
    ${SRCROOT}/cloudui.h
    ${SRCROOT}/globals.h
    ${SRCROOT}/irisandmarigold.h
    ${SRCROOT}/languagechooser.h
    ${SRCROOT}/mainpacketreceiver.h
    ${SRCROOT}/mainwindow.h
    ${SRCROOT}/multicastsetup.h
    ${SRCROOT}/packet.h
    ${SRCROOT}/packetlogmodel.h
    ${SRCROOT}/packetnetwork.h
    ${SRCROOT}/panel.h
    ${SRCROOT}/panelgenerator.h
    ${SRCROOT}/persistenthttp.h
    ${SRCROOT}/postdatagen.h
    ${SRCROOT}/sendpacketbutton.h
    ${SRCROOT}/settings.h
    ${SRCROOT}/subnetcalc.h
    ${SRCROOT}/tcpthread.h
    ${SRCROOT}/threadedtcpserver.h
    ${SRCROOT}/udpflooding.h
    ${SRCROOT}/wakeonlan.h
)

set(FORMS 
    ${SRCROOT}/about.ui
    ${SRCROOT}/brucethepoodle.ui
    ${SRCROOT}/cloudui.ui
    ${SRCROOT}/irisandmarigold.ui
    ${SRCROOT}/languagechooser.ui
    ${SRCROOT}/mainwindow.ui
    ${SRCROOT}/multicastsetup.ui
    ${SRCROOT}/panelgenerator.ui
    ${SRCROOT}/persistentconnection.ui
    ${SRCROOT}/persistenthttp.ui
    ${SRCROOT}/postdatagen.ui
    ${SRCROOT}/settings.ui
    ${SRCROOT}/subnetcalc.ui
    ${SRCROOT}/udpflooding.ui
    ${SRCROOT}/wakeonlan.ui

)

qt_add_resources(RESOURCES ${SRCROOT}/packetsender.qrc ${SRCROOT}/translations.qrc ${SRCROOT}/qdarkstyle/style.qrc)

set(CSS
    ${SRCROOT}/packetsender.css
)

qt_add_executable(packetsender 
    ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES} ${CSS}
)

execute_process(COMMAND git -C ${SRCROOT}/ rev-parse --short HEAD OUTPUT_VARIABLE GIT_SHA COMMAND_ERROR_IS_FATAL ANY OUTPUT_STRIP_TRAILING_WHITESPACE)

target_compile_definitions(packetsender PRIVATE
    GUI_BUILD
    ISSNAP=1
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
    GIT_CURRENT_SHA1=\"${GIT_SHA}\"
)


target_link_libraries(packetsender PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)



target_compile_definitions(packetsender PRIVATE
    _FORTIFY_SOURCE=2
)


install(TARGETS packetsender
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${SRCROOT}/packetsender.desktop DESTINATION /usr/share/applications)
install(FILES ${SRCROOT}/packetsender.svg DESTINATION /usr/share/icons)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
