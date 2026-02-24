// translations.h
#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include <QTranslator>
#include <QApplication>
#include <QLibraryInfo>
#include <QMap>
#include <QString>
#include <tuple>

class Translations
{
public:
    static bool installLanguage(const QString &language);

private:
    static bool loadAndInstallTranslators(
        QTranslator &qtTrans,
        QTranslator &qtbaseTrans,
        QTranslator &appTrans,
        const QString &qtName,
        const QString &qtbaseName,
        const QString &appQmPath);

    // Map: language → (qt filename, qtbase filename, app .qm path)
    static const QMap<QString, std::tuple<QString, QString, QString>> languageMap;
};

#endif // TRANSLATIONS_H