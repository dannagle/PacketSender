cmake_minimum_required(VERSION 2.8.11)
project(PacketSender)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

if(${SNAP}) 
set(QMAKE_MAJREV 6)
else()
set(QMAKE_MAJREV 5)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()


FIND_PACKAGE(QT NAMES Qt${QMAKE_MAJREV} REQUIRED COMPONENTS Core)
FIND_PACKAGE(Qt${QMAKE_MAJREV} COMPONENTS Gui Network Widgets)

set(
  PACKETSENDER_UI_HEADERS
  about.h
  brucethepoodle.h
  irisandmarigold.h
  mainpacketreceiver.h
  mainwindow.h
  panel.h
  panelgenerator.h
  persistentconnection.h
  persistenthttp.h
  postdatagen.h
  settings.h
  udpflooding.h
  languagechooser.h
  wakeonlan.h
)

set(
  PACKETSENDER_UIS
  about.ui
  brucethepoodle.ui
  irisandmarigold.ui
  cloudui.ui
  mainwindow.ui
  multicastsetup.ui
  panelgenerator.ui
  persistentconnection.ui
  persistenthttp.ui
  postdatagen.ui
  settings.ui
  subnetcalc.ui
  udpflooding.ui
  languagechooser.ui
  wakeonlan.ui
)

set(
  PACKETSENDER_SRCS
  about.cpp
  brucethepoodle.cpp
  irisandmarigold.cpp
  cloudui.cpp
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
  languagechooser.cpp
  wakeonlan.cpp
)

set(
  PACKETSENDER_QRC
  packetsender.qrc
  qdarkstyle/style.qrc
  translations.qrc
)



if(${SNAP})

    QT_ADD_RESOURCES(srcs_qrc ${PACKETSENDER_QRC})
    add_executable(
        ${PROJECT_NAME}
        ${srcs_qrc}
        ${moc_uis}
        ${moc_srcs}
        ${PACKETSENDER_SRCS}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::Widgets
    )

    execute_process(COMMAND git -C ../../../project rev-parse --short HEAD OUTPUT_VARIABLE GIT_SHA COMMAND_ERROR_IS_FATAL ANY OUTPUT_STRIP_TRAILING_WHITESPACE)
    target_compile_definitions(PacketSender PRIVATE
        GUI_BUILD
        ISSNAP=1
        QT_DISABLE_DEPRECATED_BEFORE=0x050F00
        GIT_CURRENT_SHA1=\"${GIT_SHA}\"
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _FORTIFY_SOURCE=2
    )

    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(FILES packetsender.desktop DESTINATION /usr/share/applications)
    install(FILES packetsender.svg DESTINATION /usr/share/icons)

else()

    QT5_ADD_RESOURCES(srcs_qrc ${PACKETSENDER_QRC})
    QT5_WRAP_UI(moc_uis ${PACKETSENDER_UIS})
    QT5_WRAP_CPP(moc_srcs ${PACKETSENDER_UI_HEADERS})

    add_executable(
        ${PROJECT_NAME}
        ${srcs_qrc}
        ${moc_uis}
        ${moc_srcs}
        ${PACKETSENDER_SRCS}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Network
        Qt5::Widgets
    )

    QT5_USE_MODULES(${PROJECT_NAME} Core Gui Network Widgets)

    install(TARGETS
    ${PROJECT_NAME}
    DESTINATION bin
    )

    include(GetGitRevisionDescription)
    git_describe(GIT_VERSION --tags)
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" GIT_VERSION_MAJOR "${GIT_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_MINOR "${GIT_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_PATCH "${GIT_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" GIT_VERSION_SHA1 "${GIT_VERSION}")
    set(GIT_VERSION_SHORT "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}")
    get_git_head_revision(GIT_REFSPEC GIT_HASH)

    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME} project")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A network test utility for sending / receiving TCP and UDP packets http://packetsender.com/")
    set(CPACK_PACKAGE_VERSION ${GIT_VERSION_SHORT})
    set(CPACK_PACKAGE_VERSION_MAJOR ${GIT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${GIT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH 0)
    set(CPACK_PACKAGE_CONTACT "dan@dannagle.com")

    include(CPack)
endif()


