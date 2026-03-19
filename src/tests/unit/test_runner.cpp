//
// Created by Tomas Gallucci on 3/5/26.
//

// src/tests/unit/main.cpp

#include <QtTest/Qtest.h>

#include "connectionmanager_tests.h"
#include "connection_tests.h"
#include "tcpthreadqapplicationneededtests.h"
#include "translation_tests.h"
#include "tcpthreadtests.h"

int main(int argc, char *argv[])
{
    int failures = 0;

    /* Run each test class in sequence
     * Use auto so we don't need to know the exact type
     */

    // Run GUI-dependent tests with their own QApplication
    auto runGuiTest = [&failures, &argc, &argv](QObject *testObject) {
        QApplication localApp(argc, argv);
        failures += QTest::qExec(testObject, argc, argv);
        delete testObject;
    };

    // Run pure non-GUI tests without QApplication
    auto runNonGuiTest = [&failures, argc, argv](QObject *testObject) {
        failures += QTest::qExec(testObject, argc, argv);
        delete testObject;
    };

    // Order matters: translation tests first (they install translators)
    runGuiTest(new TranslationTests());
    runGuiTest(new TcpThread_QApplicationNeeded_tests());

    // Then non-GUI or independent tests
    runNonGuiTest(new TcpThreadTests());
    runNonGuiTest(new ConnectionTests());
    runNonGuiTest(new ConnectionManagerTests());

    if (failures == 0) {
        qInfo() << "All tests passed!";
    } else {
        qWarning() << "Tests failed! Total failures:" << failures;
    }

    return failures;  // 0 = all passed, non-zero = failures
}
