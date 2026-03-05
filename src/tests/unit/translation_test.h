//
// Created by Tomas Gallucci on 3/5/26.
//

#ifndef TRANSLATION_TEST_H
#define TRANSLATION_TEST_H

#include <QtTest/QTest>

class TranslationTest : public QObject
{
    Q_OBJECT

public:
    // TranslationTest();
    // ~TranslationTest();

private slots:
    void testInstallLanguage_data();
    void testInstallLanguage();
};

#endif //TRANSLATION_TEST_H
