cmake_minimum_required(VERSION 3.16)
project(PacketSender_UnitTests)

# Find Qt6 - this is critical
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test Network)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Forced architecture" FORCE)

set(TEST_NAME "packetsender_unittests")

add_executable(${TEST_NAME}
    test_runner.cpp
    testdoubles/testtcpthreadclass.h
    testdoubles/MockSslSocket.h

    utils/testutils.cpp
    utils/calltracker.cpp

    ../../translations.cpp
    translation_tests.cpp

    ../../connections/connection.cpp
    testdoubles/connections/ConnectionTestDouble.h
    connections/connectiontests.cpp

    ../../connections/basetcpconnection.cpp
    testdoubles/connections/BaseTcpConnectionTestDouble.h
    connections/basetcpconnectiontests.cpp

    ../../connections/incomingtcpconnection.cpp
    testdoubles/connections/incomingtcpconnectiontestdouble.h
    connections/incomingtcpconnectiontests.cpp

    ../../connections/outgoingtcpconnection.cpp
    testdoubles/connections/outgoingtcpconnectiontestdouble.h
    connections/outgoingtcpconnectiontests.cpp

    ../../connectionmanager.cpp
    connectionmanager_tests.cpp

    ../../tcpthread.cpp
    ../../persistentLoopConnection.cpp
    tcpthreadtests.cpp
    tcpthreadqapplicationneededtests.cpp
    persistentconnectionlooptests.cpp

    ../../packetsenderqsslsocketinterface.h
    ../../realqsslsocket.cpp

    ../../basetcpthread.cpp
    basetcpthreadtests.cpp
    testdoubles/basetcpthreadtestdouble.h

    ../../outgoingtcpthread.cpp
    outgoingtcpthreadtests.cpp
    singlesendoutgoingtcpthreadtests.cpp
    outgoingtcpthreadpersistentconnectionlooptests.cpp
    outgoingtcpthreadconnectiontests.cpp
    testdoubles/outgoingtchpthreadtestdouble.h

    ../../incomingtcpthread.cpp
    incomingtcpthreadtests.cpp
    testdoubles/incomingtcpthreadtestdouble.h

    ../../packet.cpp
    packettests.cpp

    # project files that need to be added to the executable
    ../../sendpacketbutton.cpp
    ../../fileutils.cpp

    ../../packetsender.qrc
)

target_include_directories(${TEST_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}             # so #include "translations.h" works cleanly
)

target_link_libraries(${TEST_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets                    # needed because installLanguage uses QApplication
    Qt6::Test
    Qt6::Network
)

# Make ctest run it
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
