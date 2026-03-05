set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Forced architecture" FORCE)

set(TEST_NAME "packetsender_unittests")

add_executable(${TEST_NAME}
    test_runner.cpp

    ../../translations.cpp
    translation_tests.cpp

    ../../connection.cpp
    connection_tests.cpp
)

target_include_directories(${TEST_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}             # so #include "translations.h" works cleanly
)

target_link_libraries(${TEST_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets                    # needed because installLanguage uses QApplication
    Qt6::Test
)

# Make ctest run it
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
