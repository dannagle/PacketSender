cmake_minimum_required(VERSION 3.16)
project(PacketSender LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets)

set(
  PACKETSENDER_UI_HEADERS
  about.h
  brucethepoodle.h
  irisandmarigold.h
  mainpacketreceiver.h
  mainwindow.h
  panel.h
  globals.h
  panelgenerator.h
  persistentconnection.h
  persistenthttp.h
  postdatagen.h
  settings.h
  udpflooding.h
  languagechooser.h
  wakeonlan.h
  association.h
  dtlsserver.h
  dtlsthread.h
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
  association.cpp
  dtlsserver.cpp
  dtlsthread.cpp
)

set(
  PACKETSENDER_QRC
  packetsender.qrc
  qdarkstyle/style.qrc
  translations.qrc
)

add_executable(
  ${PROJECT_NAME}
  ${PACKETSENDER_SRCS}
  ${PACKETSENDER_UI_HEADERS}
  ${PACKETSENDER_UIS}
  ${PACKETSENDER_QRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Widgets
)

# -------------------
# Versioning from git
# -------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(GetGitRevisionDescription)
git_describe(GIT_VERSION --tags)
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" GIT_VERSION_MAJOR "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_MINOR "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_PATCH "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" GIT_VERSION_SHA1 "${GIT_VERSION}")
set(GIT_VERSION_SHORT "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}")
get_git_head_revision(GIT_REFSPEC GIT_HASH)

# -------------------
# Packaging
# -------------------
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME} project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A network test utility for sending/receiving TCP/UDP/SSL and HTTP https://packetsender.com/")
set(CPACK_PACKAGE_VERSION ${GIT_VERSION_SHORT})
set(CPACK_PACKAGE_VERSION_MAJOR ${GIT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GIT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CONTACT "dan@dannagle.com")

include(CPack)

install(TARGETS
  ${PROJECT_NAME}
  DESTINATION bin
)

# Install desktop file for snap desktop integration
install(FILES packetsender.desktop
        DESTINATION share/applications
        RENAME packetsender.desktop)

# Optional but recommended: install scalable SVG icon
install(FILES packetsender.svg
        DESTINATION share/icons/hicolor/scalable/apps)

