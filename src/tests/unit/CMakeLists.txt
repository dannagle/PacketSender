set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Forced architecture" FORCE)

set(TEST_NAME "translation_test")

add_executable(${TEST_NAME}
    translation_test.cpp

    ../../translations.h
    ../../translations.cpp
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
