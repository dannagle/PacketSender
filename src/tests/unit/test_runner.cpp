//
// Created by Tomas Gallucci on 8/8/26.
//


#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <qtestcase.h>
#include <qxmlstream.h>

#include "connections/connectiontests.h"
#include "connections/basetcpconnectiontests.h"
#include "connectionmanager_tests.h"
#include "translation_tests.h"
#include "tcpThreads/basetcpthreadtests.h"
#include "tcpThreads/incomingtcpthreadtests.h"
#include "tcpThreads/outgoingtcpthreadpersistentconnectionlooptests.h"
#include "tcpThreads/outgoingtcpthreadconnectiontests.h"
#include "tcpThreads/outgoingtcpthreadtests.h"
#include "packettests.h"
#include "tcpThreads/singlesendoutgoingtcpthreadtests.h"
#include "connections/incomingtcpconnectiontests.h"
#include "connections/outgoingtcpconnectiontests.h"

struct TestSummary {
    int total = 0;
    int failures = 0;
    int skipped = 0;
    int errors = 0;

    TestSummary &operator+=(const TestSummary &other) {
        total    += other.total;
        failures += other.failures;
        skipped  += other.skipped;
        errors   += other.errors;
        return *this;
    }
};

TestSummary parseJUnitXml(const QString &fileName)
{
    TestSummary summary;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return summary;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("testsuite")) {
            summary.total    += xml.attributes().value("tests").toInt();
            summary.failures += xml.attributes().value("failures").toInt();
            summary.skipped  += xml.attributes().value("skipped").toInt();
            summary.errors   += xml.attributes().value("errors").toInt();
        }
    }
    return summary;
}

// Helper that runs after every qExec
void finishTest(QObject *testObject,
                const QString &reportFile,
                TestSummary &grandTotal)
{
    TestSummary s = parseJUnitXml(reportFile);
    grandTotal += s;

    // qInfo().noquote() << QString("Totals: %1 passed, %2 failed, %3 skipped, %4 blacklisted")
    //                         .arg(s.total - s.failures - s.skipped - s.errors)
    //                         .arg(s.failures)
    //                         .arg(s.skipped)
    //                         .arg(0);

    delete testObject;
}

QStringList buildTestArgs(int argc, char *argv[], const QString &reportFile)
{
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args << QString::fromLocal8Bit(argv[i]);

    // Keep normal console output
    args << "-o" << "-,txt";

    // Machine-readable report
    args << "-o" << reportFile + ",junitxml";

    return args;
}

int main(int argc, char *argv[])
{
    int totalFailures = 0;
    int testsRun = 0;
    TestSummary grandTotal;
    QStringList reportFiles;

    const QString reportDir = "test-results";

    // Clean up from previous runs; mostly for running tests over and over locally, not for CI pipeline
    QDir dir(reportDir);
    if (dir.exists()) {
        if (!dir.removeRecursively())
        {
            throw std::runtime_error("Could not delete existing test report directory");
        }
    }
    if (! QDir().mkpath(reportDir))
    {
        throw std::runtime_error("Could not create test report directory");
    }

    auto runGuiTest = [&](QObject *testObject) {
        const QString reportFile = QString("%1/test-results-%2.xml")
                                        .arg(reportDir)
                                        .arg(++testsRun, 2, 10, QChar('0'));

        QStringList args = buildTestArgs(argc, argv, reportFile);

        QApplication localApp(argc, argv);
        totalFailures += QTest::qExec(testObject, args);
        finishTest(testObject, reportFile, grandTotal);
    };

    auto runNonGuiTest = [&](QObject *testObject) {
        const QString reportFile = QString("%1/test-results-%2.xml")
                                        .arg(reportDir)
                                        .arg(++testsRun, 2, 10, QChar('0'));
        QStringList args = buildTestArgs(argc, argv, reportFile);

        QCoreApplication app(argc, argv);
        totalFailures += QTest::qExec(testObject, args);
        finishTest(testObject, reportFile, grandTotal);
    };

    runGuiTest(new TranslationTests());
    runGuiTest(new BaseTcpThreadTests());
    runGuiTest(new OutgoingTcpThreadTests());
    runGuiTest(new SingleSendOutgoingTcpThreadTests());
    runGuiTest(new OutgoingTcpThreadPersistentConnectionLoopTests());
    runGuiTest(new OutgoingTcpThreadConnectionTests());
    runGuiTest(new IncomingTcpThreadTests());
    runGuiTest(new ConnectionManagerTests());

    runNonGuiTest(new PacketTests());
    runNonGuiTest(new BaseTcpConnectionTests());
    runNonGuiTest(new IncomingTcpConnectionTests());
    runNonGuiTest(new OutgoingTcpConnectionTests());
    runNonGuiTest(new ConnectionTests());
    // ---------------------------------------------------------------

    // Final summary
    qInfo().noquote() << "\n========================================";
    qInfo().noquote() << "Test run finished:" << QDateTime::currentDateTime().toString(Qt::ISODate);
    qInfo().noquote() << QString("Test classes executed : %1").arg(testsRun);
    qInfo().noquote() << QString("Total tests           : %1").arg(grandTotal.total);
    qInfo().noquote() << QString("Failures              : %1").arg(grandTotal.failures);
    qInfo().noquote() << QString("Skipped               : %1").arg(grandTotal.skipped);
    qInfo().noquote() << QString("Errors                : %1").arg(grandTotal.errors);
    qInfo().noquote() << "========================================";

    if (grandTotal.failures == 0 && grandTotal.errors == 0)
        qInfo() << "All tests passed!";
    else
        qWarning() << "Some tests failed or had errors!";

    return (grandTotal.failures + grandTotal.errors) > 0 ? 1 : 0;
}
