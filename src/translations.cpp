// translations.cpp
#include "translations.h"
#include <QDebug>
#include "globals.h"
// Define the static map

#ifdef __APPLE__
const QMap<QString, std::tuple<QString, QString, QString>> Translations::languageMap = {
    {"Chinese",  {"qt_zh_CN",  "qtbase_zh_CN",  "packetsender_cn.qm"}},
    {"Spanish",  {"qt_es",     "qtbase_es",     "packetsender_es.qm"}},
    {"German",   {"qt_de",     "qtbase_de",     "packetsender_de.qm"}},
    {"French",   {"qt_fr",     "qtbase_fr",     "packetsender_fr.qm"}},
    {"Italian",  {"qt_it",     "qtbase_it",     "packetsender_it.qm"}},
};
#else

const QMap<QString, std::tuple<QString, QString, QString>> Translations::languageMap = {
    {"Chinese",  {"qt_zh_CN",  "qtbase_zh_CN",  ":/languages/packetsender_cn.qm"}},
    {"Spanish",  {"qt_es",     "qtbase_es",     ":/languages/packetsender_es.qm"}},
    {"German",   {"qt_de",     "qtbase_de",     ":/languages/packetsender_de.qm"}},
    {"French",   {"qt_fr",     "qtbase_fr",     ":/languages/packetsender_fr.qm"}},
    {"Italian",  {"qt_it",     "qtbase_it",     ":/languages/packetsender_it.qm"}},
    };

#endif



bool Translations::loadAndInstallTranslators(
    QTranslator &qtTrans,
    QTranslator &qtbaseTrans,
    QTranslator &appTrans,
    const QString &qtName,
    const QString &qtbaseName,
    const QString &appQmPath)
{

#ifdef __APPLE__
    QString translationDir = QCoreApplication::applicationDirPath() + "/../Resources/languages/";
    bool qtOk     = qtTrans.load(qtName,     translationDir);
    bool qtbaseOk = qtbaseTrans.load(qtbaseName, translationDir);
    bool appOk    = appTrans.load(translationDir + appQmPath);

#else
    QString translationDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    bool qtOk     = qtTrans.load(qtName,     translationDir);
    bool qtbaseOk = qtbaseTrans.load(qtbaseName, translationDir);
    bool appOk    = appTrans.load(appQmPath);
#endif

    QDEBUG() << "Attempting"     << qtName << qtbaseName << appQmPath << translationDir;    
    QDEBUG() << "qt lang loaded"     << qtOk;
    QDEBUG() << "base lang loaded"   << qtbaseOk;
    QDEBUG() << "app lang loaded"    << appOk;
    
    bool installed1 = QApplication::installTranslator(&qtTrans);
    bool installed2 = QApplication::installTranslator(&qtbaseTrans);
    bool installed3 = QApplication::installTranslator(&appTrans);
    
    QDEBUGVAR(installed1);
    QDEBUGVAR(installed2);
    QDEBUGVAR(installed3);
    
    bool allInstalled = installed1 && installed2 && installed3;

    QDEBUG() << "All translators installed:" << allInstalled;

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