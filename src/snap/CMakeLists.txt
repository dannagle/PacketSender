cmake_minimum_required(VERSION 3.16)
project(packetsender VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network Widgets)

set(SOURCES
    about.cpp
    brucethepoodle.cpp
    cloudui.cpp
    irisandmarigold.cpp
    languagechooser.cpp
    main.cpp
    mainpacketreceiver.cpp
    mainwindow.cpp
    multicastsetup.cpp
    packet.cpp
    packetlogmodel.cpp
    packetnetwork.cpp
    panel.cpp
    panelgenerator.cpp
    persistentconnection.cpp
    persistenthttp.cpp
    postdatagen.cpp
    sendpacketbutton.cpp
    settings.cpp
    subnetcalc.cpp
    tcpthread.cpp
    threadedtcpserver.cpp
    udpflooding.cpp
    wakeonlan.cpp
)

set(HEADERS 
    about.h
    brucethepoodle.h
    cloudui.h
    globals.h
    irisandmarigold.h
    languagechooser.h
    mainpacketreceiver.h
    mainwindow.h
    multicastsetup.h
    packet.h
    packetlogmodel.h
    packetnetwork.h
    panel.h
    panelgenerator.h
    persistenthttp.h
    postdatagen.h
    sendpacketbutton.h
    settings.h
    subnetcalc.h
    tcpthread.h
    threadedtcpserver.h
    udpflooding.h
    wakeonlan.h
)

set(FORMS 
    about.ui
    brucethepoodle.ui
    cloudui.ui
    irisandmarigold.ui
    languagechooser.ui
    mainwindow.ui
    multicastsetup.ui
    panelgenerator.ui
    persistentconnection.ui
    persistenthttp.ui
    postdatagen.ui
    settings.ui
    subnetcalc.ui
    udpflooding.ui
    wakeonlan.ui

)

qt_add_resources(RESOURCES packetsender.qrc translations.qrc qdarkstyle/style.qrc)

set(CSS
    packetsender.css
)

qt_add_executable(packetsender 
    ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES} ${CSS}
)

execute_process(COMMAND git -C  ${PROJECT_SOURCE_DIR} rev-parse --short HEAD OUTPUT_VARIABLE GIT_SHA COMMAND_ERROR_IS_FATAL ANY OUTPUT_STRIP_TRAILING_WHITESPACE)

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

install(FILES packetsender.desktop DESTINATION /usr/share/applications)
install(FILES packetsender.svg DESTINATION /usr/share/icons)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
