//
// Created by Tomas Gallucci on 4/3/26.
//

#include <QtTest/QTest.h>
#include <QSignalSpy>

#include "testutils.h"


void TestUtils::debugSpy(const QSignalSpy& spy)
{
    qDebug() << "QSignalSpy captured" << spy.count() << "emissions";
    for (int i = 0; i < spy.count(); ++i) {
        QList<QVariant> args = spy.at(i);
        qDebug() << "  Emission" << i << "has" << args.size() << "arguments:";
        for (int j = 0; j < args.size(); ++j) {
            qDebug() << "    Arg" << j << ":" << args.at(j)
                     << "(type:" << args.at(j).typeName() << ")";
        }
    }
}
