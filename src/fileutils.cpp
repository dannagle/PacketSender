//
// Created by Tomas Gallucci on 6/2/26.
//

#include <QString>
#include <QDebug>
#include "fileutils.h"

bool FileUtils::openFileInReadOnlyMode(QFile& file, const QString& filePath)
{
    qDebug().noquote() << "filepath: " << filePath;
    file.setFileName(filePath);
    if (file.open(QIODevice::ReadOnly)&& file.size() > 0) {
        return true;
    }

    qWarning() << fileErrorMessage(file);
    return false;
}

QByteArray FileUtils::decodeBase64EncodedResourceFile(const QString& filePath)
{
    qDebug().noquote() << "base64 filepath: " << filePath;
    QFile file;
    file.setFileName(filePath);
    if (file.open(QIODevice::ReadOnly)&& file.size() > 0) {
        return QByteArray::fromBase64(file.readAll());
    }

    qWarning() << fileErrorMessage(file);
    return QByteArray();
}

QString FileUtils::fileErrorMessage(const QFile& file)
{
    return QString("Failed to open %1: %2")
                .arg(file.fileName())
                .arg(file.errorString());
}
