//
// Created by Tomas Gallucci on 3/2/26.
//

#include <QtTest/QtTest>

// test headers
#include "translation_test.h"

// code headers
#include "translations.h"


void TranslationTest::testInstallLanguage_data()
{
    QTest::addColumn<QString>("language");
    QTest::addColumn<bool>("expected");

    QTest::newRow("empty → system default")     << ""         << true;
    QTest::newRow("unsupported")                << "Klingon" << false;
    QTest::newRow("Spanish (supported)")        << "Spanish"  << true;
    QTest::newRow("German (supported)")         << "German"   << true;
    QTest::newRow("French (supported)")         << "French"   << true;
    QTest::newRow("Italian (supported)")        << "Italian"  << true;
    QTest::newRow("Chinese (supported)")        << "Chinese"  << true;
}

void TranslationTest::testInstallLanguage()
{
    QFETCH(QString, language);
    QFETCH(bool, expected);

    // This exercises both loadAndInstallTranslators() and the map lookup
    bool result = Translations::installLanguage(language);

    QCOMPARE(result, expected);
}

// QTEST_MAIN(TranslationTest)
// #include "translation_test.moc"
