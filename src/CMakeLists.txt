cmake_minimum_required(VERSION 2.8.11)
project(PacketSender)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

FIND_PACKAGE(Qt5 COMPONENTS Core Gui Network Widgets)

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
)

set(
  PACKETSENDER_QRC
  packetsender.qrc
  qdarkstyle/style.qrc
  translations.qrc
)

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

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Core)
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



