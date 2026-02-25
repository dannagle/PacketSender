// translations.cpp
#include "translations.h"
#include <QDebug>

// Define the static map
const QMap<QString, std::tuple<QString, QString, QString>> Translations::languageMap = {
    {"Chinese",  {"qt_zh_cn",     "qtbase_zh_cn",     ":/languages/packetsender_cn.qm"}},
    {"Spanish",  {"qt_es",         "qtbase_es",         ":/languages/packetsender_es.qm"}},
    {"German",   {"qt_de",         "qtbase_de",         ":/languages/packetsender_de.qm"}},
    {"French",   {"qt_fr",         "qtbase_fr",         ":/languages/packetsender_fr.qm"}},
    {"Italian",  {"qt_it",         "qtbase_it",         ":/languages/packetsender_it.qm"}},
};

bool Translations::loadAndInstallTranslators(
    QTranslator &qtTrans,
    QTranslator &qtbaseTrans,
    QTranslator &appTrans,
    const QString &qtName,
    const QString &qtbaseName,
    const QString &appQmPath)
{
    bool qtOk     = qtTrans.load(qtName,     QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    bool qtbaseOk = qtbaseTrans.load(qtbaseName, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    bool appOk    = appTrans.load(appQmPath);

    qDebug() << "qt lang loaded"     << qtOk;
    qDebug() << "base lang loaded"   << qtbaseOk;
    qDebug() << "app lang loaded"    << appOk;

    bool allInstalled =
        QApplication::installTranslator(&qtTrans) &&
        QApplication::installTranslator(&qtbaseTrans) &&
        QApplication::installTranslator(&appTrans);

    qDebug() << "All translators installed:" << allInstalled;

    return allInstalled;
}

bool Translations::installLanguage(const QString &language)
{
    if (language.isEmpty()) {
        qDebug() << "No language selected — using system default";
        return true;
    }

    if (!languageMap.contains(language)) {
        qDebug() << "Unsupported language:" << language << "— using system default";
        return false;
    }

    auto [qtName, qtbaseName, appQm] = languageMap[language];  // C++17 structured binding

    static QTranslator qtTrans, qtbaseTrans, appTrans;

    return loadAndInstallTranslators(
        qtTrans, qtbaseTrans, appTrans,
        qtName, qtbaseName, appQm
    );
}