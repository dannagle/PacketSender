//
// Created by Tomas Gallucci on 6/2/26.
//

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QFile>
#include <QString>

class FileUtils
{
public:
    static bool openFileInReadOnlyMode(QFile& file, const QString& filePath);
    static QByteArray decodeBase64EncodedResourceFile(const QString& filePath);
    static QString fileErrorMessage(const QFile& file);
};


#endif //FILEUTILS_H
