//
// Created by Tomas Gallucci on 3/5/26.
//

// src/tests/unit/main.cpp

#include <QtTest/Qtest.h>

#include "connection_tests.h"
#include "translation_test.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;

    // Run each test class in sequence
    // Use auto so we don't need to know the exact type
    auto runTest = [&status, argc, argv](QObject *testObject) -> void {
        status |= QTest::qExec(testObject, argc, argv);
    };

    // Instantiate and run each
    runTest(new TranslationTests());
    runTest(new ConnectionTests());
    // Add new ones here as you go, e.g. runTest(new TestSomethingElse());

    return status;  // 0 = all passed, non-zero = failures
}
