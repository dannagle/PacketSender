/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#include <QtWidgets/QApplication>
#include <QDir>
#include <QDesktopServices>
#include <QCommandLineParser>
#include <QHostInfo>
#include <QSslError>
#include <QList>
#include <QSslCipher>
#include <QSslSocket>
#include <QDeadlineTimer>

#include "panelgenerator.h"
#define DEBUGMODE 0


void myMessageOutputDisable(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    Q_UNUSED(type);
    Q_UNUSED(context);
    Q_UNUSED(msg);
}

#define OUTVAR(var)  o<< "\n" << # var << ":" << var ;
#define OUTIF()  if(!quiet) o<< "\n"
#define OUTPUT() outBuilder = outBuilder.trimmed(); outBuilder.append("\n"); out << outBuilder; outBuilder.clear();


int main(int argc, char *argv[])
{
    int debugMode = DEBUGMODE;

    if (QFile::exists("DEBUGMODE")) {
        debugMode = 1;
    }

    if (QFile::exists(QDir::homePath() + "/DEBUGMODE")) {
        debugMode = 1;
    }


    if (debugMode) {
        QDEBUG() << "run-time debug mode";

    } else {
        qInstallMessageHandler(myMessageOutputDisable);

    }

    QDEBUG() << "number of arguments:" << argc;
    QStringList args;
    QDEBUGVAR(RAND_MAX);



    bool gatekeeper = false;


    //Upon first launch, Apple will assign a psn number and
    //pass it as a command line argument.

    //This is most often during the gatekeeper stage.

    //It only does this on first launch. I still need to catch it though.

    if (argc == 2) {

        gatekeeper = true;

        QString arg2 = QString(argv[1]);

        //only the help and version should trigger
        if (arg2.contains("-h")) {
            gatekeeper = false;
        }
        if (arg2.contains("help")) {
            gatekeeper = false;
        }
        if (arg2.contains("-v")) {
            gatekeeper = false;
        }
        if (arg2.contains("version")) {
            gatekeeper = false;
        }

    }



    if ((argc > 1) && !gatekeeper) {
        QCoreApplication a(argc, argv);
        args = a.arguments();

        QDEBUGVAR(args);

        qRegisterMetaType<Packet>();



        QDEBUG() << "Running command line mode.";

        Packet sendPacket;
        sendPacket.init();
        QString outBuilder;

        QTextStream o(&outBuilder);


        QTextStream out(stdout);

        QCoreApplication::setApplicationName("Packet Sender Studio");
        QString versionBuilder = QString("version ") + SW_VERSION;
        if (QSslSocket::supportsSsl()) {
            versionBuilder.append(" / SSL version:");
            versionBuilder.append(QSslSocket::sslLibraryBuildVersionString());
        }
        QCoreApplication::setApplicationVersion(versionBuilder);

        QCommandLineParser parser;
        parser.setApplicationDescription("Packet Sender is a Network UDP/TCP/SSL Test Utility by NagleCode\nSee https://PacketSender.com/ for more information.");
        parser.addHelpOption();
        parser.addVersionOption();


        // Process the actual command line arguments given by the user
        parser.process(a);

    } else {


        QApplication a(argc, argv);

        QDEBUGVAR(args);

        qRegisterMetaType<Packet>();


        //Use default OS styling for non-Windows. Too many theme variants.

        QFile file(":/qdarkstyle/style.qss");
        if (file.open(QFile::ReadOnly)) {
            QString StyleSheet = QLatin1String(file.readAll());
            //  qDebug() << "stylesheet: " << StyleSheet;
            a.setStyleSheet(StyleSheet);
        }


        PanelGenerator w;
        w.initAutoLaunch();


        w.show();

        return a.exec();

    }



    return 0;

}
