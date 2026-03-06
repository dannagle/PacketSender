//
// Created by Tomas Gallucci on 3/5/26.
//

// src/tests/unit/main.cpp

#include <QtTest/Qtest.h>

#include "connection_tests.h"
#include "translation_tests.h"

int main(int argc, char *argv[])
{
    int status = 0;

    /* Run each test class in sequence
     * Use auto so we don't need to know the exact type
     */

    // Run GUI-dependent tests with their own QApplication
    auto runGuiTest = [&status, &argc, &argv](QObject *testObject) {
        QApplication localApp(argc, argv);
        status |= QTest::qExec(testObject, argc, argv);
        delete testObject;
    };

    // Run pure non-GUI tests without QApplication
    auto runNonGuiTest = [&status, argc, argv](QObject *testObject) {
        status |= QTest::qExec(testObject, argc, argv);
        delete testObject;
    };

    // Order matters: translation tests first (they install translators)
    runGuiTest(new TranslationTests());

    // Then non-GUI or independent tests
    runNonGuiTest(new ConnectionTests());

    return status;  // 0 = all passed, non-zero = failures
}
