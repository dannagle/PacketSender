/*
 *
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * https://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */

#include "globals.h"
#ifndef CONSOLE_BUILD
    #include <QtWidgets/QApplication>
    #include <QDesktopServices>
    #include <QTranslator>
    #include <QLibraryInfo>
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    #include <QGuiApplication>
    #include <QStyleHints>
#endif

    #include "settings.h"
    #include "languagechooser.h"
#endif
#include <QDir>
#include <QCommandLineParser>
#include <QHostInfo>
#include <QSslError>
#include <QList>
#include <QSslCipher>
#include <QDeadlineTimer>
#include <QProcess>
#include <QStandardPaths>
#include <QSettings>

#include "mainpacketreceiver.h"

#ifndef CONSOLE_BUILD
    #include "panelgenerator.h"
    #include "packetnetwork.h"
    #include "mainwindow.h"
#endif
#define DEBUGMODE 0



#define STOPSENDCHECK()     if(hasstop) { \
                                stopcounter++; \
                                if(stopcounter >= stopnum) { \
                                    break; \
                                } \
                            }



int intenseTrafficGenerator(QTextStream &out, QUdpSocket &sock, QHostAddress addy, unsigned int port, QString hexString, double bps, double rate, qint64 stopnum, qint64 usdelay);



bool isGuiApp()
{

    QProcess *process = new QProcess();
    QString program = "xrandr";
    process->start(program, QStringList());
    process->waitForFinished(500);
    int exitcode = process->exitCode();
    QDEBUGVAR(exitcode);
    free(process);
    if (exitcode > 0) {
        // This means xrandr exists, but it couldn't connect.
        return false;
    }

    if (exitcode < 0) {
        //command not found. Maybe xrandr isn't present.
        // TODO some other test?
        return true;
    }

    // returned zero. All is good.
    return true;
}

void myMessageOutputDisable(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    Q_UNUSED(type);
    Q_UNUSED(context);
    Q_UNUSED(msg);
}

#define OUTVAR(var)  o<< "\n" << # var << ":" << var ;
#define OUTIF()  if(!quiet) o<< "\n"
#define OUTPUT() outBuilder = outBuilder.trimmed(); outBuilder.append("\n"); out << outBuilder; out.flush(); outBuilder.clear();




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

    QDEBUGVAR(QThread::idealThreadCount());


    bool gatekeeper = false;

    bool force_gui = false;
    bool panels_only = false;

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
        if (arg2.contains("-l")) {
            gatekeeper = false;
        }
        if (arg2.contains("version")) {
            gatekeeper = false;
        }

        force_gui = arg2.contains("--gui");
        panels_only = arg2.contains("--starterpanel");

    }

    //Create the settings folders if they do not exist
    if(!QFile::exists("portablemode.txt")) {
        QDir mdir;
        mdir.mkpath(SETTINGSPATH);
    }

    //this is stored as base64 so smart git repos
    //do not complain about shipping a private key.
    QFile snakeoilKey("://ps.key.base64");
    QFile snakeoilCert("://ps.pem.base64");


    QString defaultCertFile = CERTFILE;
    QString defaultKeyFile = KEYFILE;

    QFile certfile(defaultCertFile);
    QFile keyfile(defaultKeyFile);
    QByteArray decoded;
    decoded.clear();

    if (!certfile.exists()) {
        if (snakeoilCert.open(QFile::ReadOnly)) {
            decoded = QByteArray::fromBase64(snakeoilCert.readAll());
            snakeoilCert.close();
        }
        if (certfile.open(QFile::WriteOnly)) {
            certfile.write(decoded);
            certfile.close();
        }
    }

    if (!keyfile.exists()) {
        if (snakeoilKey.open(QFile::ReadOnly)) {
            decoded = QByteArray::fromBase64(snakeoilKey.readAll());
            snakeoilKey.close();
        }
        if (keyfile.open(QFile::WriteOnly)) {
            keyfile.write(decoded);
            keyfile.close();
        }
    }

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    if(settings.value("leaveSessionOpen", "").toString().isEmpty()) {
        settings.setValue("leaveSessionOpen", "false");
    }





#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    srand(time(NULL));
#endif

    if (((argc > 1) && (!gatekeeper))

#ifdef CONSOLE_BUILD
    || true
#endif
    ) {
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

        QCoreApplication::setApplicationName("Packet Sender");
        QString versionBuilder = QString("Version ") + SW_VERSION;
        if (QSslSocket::supportsSsl()) {
            versionBuilder.append(" / SSL:");
            versionBuilder.append(QSslSocket::sslLibraryBuildVersionString());
        } else {
            versionBuilder.append(" / SSL library not found");
        }

        #ifdef GIT_CURRENT_SHA1
            versionBuilder.append(" / Commit: " + QString(GIT_CURRENT_SHA1));
        #endif

        QCoreApplication::setApplicationVersion(versionBuilder);

        QCommandLineParser parser;
        parser.setApplicationDescription("Packet Sender is a Network UDP/TCP/SSL/HTTP Test Utility by NagleCode\nSee https://PacketSender.com/ for more information.");
        parser.addHelpOption();
        parser.addVersionOption();


        // A boolean option with a single name (-p)
        QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Quiet mode. Only output received data.");
        parser.addOption(quietOption);


        QCommandLineOption hexOption(QStringList() << "x" << "hex", "Parse data-to-send as hex (default for TCP/UDP/SSL).");
        parser.addOption(hexOption);

        QCommandLineOption asciiOption(QStringList() << "a" << "ascii", "Parse data-to-send as mixed-ascii (default for http and GUI).");
        parser.addOption(asciiOption);

        QCommandLineOption pureAsciiOption(QStringList() << "A" << "ASCII", "Parse data-to-send as pure ascii (no \\xx translation).");
        parser.addOption(pureAsciiOption);


        // Command line server mode
        QCommandLineOption serverOption(QStringList() << "l" << "listen", "Listen instead of send. Use bind options to specify port/IP. Otherwise, dynamic/All.");
        parser.addOption(serverOption);

        // Command line server response
        QCommandLineOption responseOption(QStringList() << "r" << "response", "Server mode response data in mixed-ascii. Macro supported.", "ascii");
        parser.addOption(responseOption);


        // An option with a value
        QCommandLineOption waitOption(QStringList() << "w" << "wait",
                                      "Wait up to <milliseconds> for a response after sending. Zero means do not wait (Default).",
                                      "ms");
        parser.addOption(waitOption);

        // An option with a value
        QCommandLineOption fileOption(QStringList() << "f" << "file",
                                      "Send contents of specified path. Max 10 MiB for UDP, 100 MiB for TCP/SSL.",
                                      "path");
        parser.addOption(fileOption);


        // An option with a value
        QCommandLineOption bindPortOption(QStringList() << "b" << "bind",
                                          "Bind port. Default is 0 (dynamic).",
                                          "port");
        parser.addOption(bindPortOption);


        QCommandLineOption bindIPv6Option(QStringList() << "6" << "ipv6", "Force IPv6. Same as -B \"::\". Default is IP:Any.");
        parser.addOption(bindIPv6Option);
        QCommandLineOption bindIPv4Option(QStringList() << "4" << "ipv4", "Force IPv4.  Same as -B \"0.0.0.0\". Default is IP:Any.");
        parser.addOption(bindIPv4Option);
        QCommandLineOption bindIPOption(QStringList() << "B" << "bindip",
                                          "Bind custom IP. Default is IP:Any.",
                                          "IP");
        parser.addOption(bindIPOption);
        QCommandLineOption tcpOption(QStringList() << "t" << "tcp", "Send TCP (default).");
        parser.addOption(tcpOption);

        QCommandLineOption sslOption(QStringList() << "s" << "ssl", "Send SSL and ignore errors.");
        parser.addOption(sslOption);

        QCommandLineOption sslNoErrorOption(QStringList() << "S" << "SSL", "Send SSL and stop for errors.");
        parser.addOption(sslNoErrorOption);

        // A boolean option with multiple names (-f, --force)
        QCommandLineOption udpOption(QStringList() << "u" << "udp", "Send UDP.");
        parser.addOption(udpOption);


        QCommandLineOption dtlsOption(QStringList() << "dtls", "Send DTLS.");
        if(PacketNetwork::DTLSisSupported()) {
            //parser.addOption(dtlsOption);
            QDEBUG() << "DTLS is not yet supported on DTLS.";
        }

        // A single option with a value
        QCommandLineOption httpOption(QStringList() << "http", "Send HTTP. Allowed values are GET (default) and POST", "http");
        parser.addOption(httpOption);

        // An option with a value
        QCommandLineOption nameOption(QStringList() << "n" << "name",
                                      "Send previously saved packet named <name>. Other options overrides saved packet parameters.",
                                      "name");
        parser.addOption(nameOption);

        QCommandLineOption wolOption(QStringList() << "wol", "Send Wake-On-LAN / Magic Packet to <mac> and (optional) <port>.", "mac");
        parser.addOption(wolOption);



        // Intense Traffic Generator
        QCommandLineOption bpsOption(QStringList() << "bps", "Intense traffic. Calculate rate based on value of bits per second.", "bps");
        parser.addOption(bpsOption);
        QCommandLineOption numOption(QStringList() << "num", "Intense traffic. Number of packets to send. Default unlimited.", "number");
        parser.addOption(numOption);
        QCommandLineOption rateOption(QStringList() << "rate", "Intense traffic. Rate. Ignored in bps option.", "Hertz");
        parser.addOption(rateOption);
        QCommandLineOption usdelayOption(QStringList() << "usdelay", "Intense traffic. Resend delay. Used if rate is 0. Ignored in bps option.", "microseconds");
        parser.addOption(usdelayOption);
        QCommandLineOption maxOption(QStringList() << "max", "Intense traffic. Run as fast as possible.");
        parser.addOption(maxOption);



        parser.addPositionalArgument("address", "Destination address/URL. Optional for saved packet.");
        parser.addPositionalArgument("port", "Destination port/POST data. Optional for saved packet.");
        parser.addPositionalArgument("data", "Data to send. Optional for saved packet.");

        if (argc < 2) {
            parser.showHelp();
            return 0;
        }

        // Process the actual command line arguments given by the user
        parser.process(a);

        const QStringList args = parser.positionalArguments();

        bool quiet = parser.isSet(quietOption);
        bool hex = parser.isSet(hexOption);
        bool mixedascii = parser.isSet(asciiOption);
        bool ascii = parser.isSet(pureAsciiOption);
        unsigned int wait = parser.value(waitOption).toUInt();
        unsigned int bind = parser.value(bindPortOption).toUInt();
        QHostAddress bindIP = QHostAddress(QHostAddress::Any);
        QDEBUGVAR(parser.isSet(bindIPOption));
        QString bindIPstr = "";
        if(parser.isSet(bindIPOption)) {
            bindIPstr = parser.value(bindIPOption).trimmed();
        }
        bool tcp = parser.isSet(tcpOption);
        bool udp = parser.isSet(udpOption);
        bool dtls = parser.isSet(dtlsOption);

        if(dtls) {
            if(!PacketNetwork::DTLSisSupported()) {
                OUTIF() << "DTLS is not supported in this installation. ";
                OUTPUT();
                return -1;
            }
        }


        bool ssl = parser.isSet(sslOption);
        bool sslNoError = parser.isSet(sslNoErrorOption);

        if(ssl) {
            if(!QSslSocket::supportsSsl()) {
                OUTIF() << "SSL is not supported in this installation. ";
                OUTPUT();
                return -1;
            }
        }



        bool ipv6  = parser.isSet(bindIPv6Option);
        bool ipv4  = parser.isSet(bindIPv4Option);
        bool http  = parser.isSet(httpOption);

        bool wol = parser.isSet(wolOption);

        bool server = parser.isSet(serverOption);
        QString response = parser.value(responseOption);
        QDEBUGVAR(response);
        bool okbps = false;
        bool okrate = false;
        bool maxrate = parser.isSet(maxOption);
        bool intense = parser.isSet(bpsOption) || parser.isSet(numOption)|| parser.isSet(rateOption) || parser.isSet(usdelayOption) || maxrate;
        double bps = parser.value(bpsOption).toDouble(&okbps);
        qint64 stopnum = parser.value(numOption).toULongLong();
        double rate = parser.value(rateOption).toDouble(&okrate);
        qint64 usdelay = parser.value(usdelayOption).toULongLong();
        if(intense) {
            if (maxrate) {
                OUTIF() << "Maximum sending. Calculated " << QThread::idealThreadCount() << " send threads are supported.";
                bps = 0;
                rate = 0;
            } else {
                if (!okrate && !okbps) {
                    OUTIF() << "Warning: Invalid rate and/or bps. Intense traffic will free-run.";
                    bps = 0;
                    rate = 0;
                }
            }
        }

        if (sslNoError) ssl = true;


        QString name = parser.value(nameOption);

        QString httpMethod = parser.value(httpOption).trimmed().toUpper();



        QString filePath = parser.value(fileOption);


        QString address = "";
        QString addressOriginal = "";
        unsigned int port = 0;

        int argssize = args.size();
        QDEBUGVAR(argssize);
        QString data, dataString;
        data.clear();
        dataString.clear();
        if (argssize >= 1) {
            address = args[0];
        }


        if(wol) {
            QString targetMAC = parser.value(wolOption).trimmed().toUpper();
            if (argssize >= 1) {
                port = QString(args[0]).toUInt();
            }
            if(port < 1) {
                port = 7;
            }

            Packet wolPkt = Packet::generateWakeOnLAN(targetMAC, port);

            if(wolPkt.errorString.isEmpty()) {
                OUTIF() << "Sending broadcast Wake-On-LAN to target: " + targetMAC + " on port " + QString::number(port);
                udp = true;
                tcp = false;
                ssl = false;
                http = false;
                address = wolPkt.toIP;
                data = wolPkt.hexString;

            } else {
                OUTIF() << "Error generating Wake-On-LAN: " + wolPkt.errorString;
                OUTPUT();
                return -1;
            }

        }


        if(http) {
            if(parser.isSet(httpOption)) {
                if(httpMethod != "GET" && httpMethod != "POST") {
                    OUTIF() << "Error: supported HTTP methods are GET and POST. Specified: " << httpMethod;
                    filePath.clear();
                    OUTPUT();
                    return -1;
                }

                if(name.isEmpty()) {
                    if(address.isEmpty()) {
                        OUTIF() << "Error: URL is required after HTTP method is no name is supplied.";
                        filePath.clear();
                        OUTPUT();
                        return -1;
                    }
                }

                if(httpMethod == "POST") {
                    if(argssize < 2) {
                        OUTIF() << "Error: data is required after specifying POST.";
                        filePath.clear();
                        OUTPUT();
                        return -1;
                    } else {
                        data = args[1];
                    }
                }

            }

        } else {
            if (argssize >= 2) {
                port = args[1].toUInt();
            }
            if (argssize >= 3) {
                data = (args[2]);
            }
        }

        bool multicast = PacketNetwork::isMulticast(address);

        //check for invalid options..

        if (argssize > 3) {
            OUTIF() << "Warning: Extra parameters detected. Try surrounding your data with quotes.";
        }

        if (hex && mixedascii) {
            OUTIF() << "Warning: both hex and pure ascii set. Defaulting to hex.";
            mixedascii = false;
        }

        if (hex && ascii) {
            OUTIF() << "Warning: both hex and pure ascii set. Defaulting to hex.";
            ascii = false;
        }

        if (mixedascii && ascii) {
            OUTIF() << "Warning: both mixed ascii and pure ascii set. Defaulting to pure ascii.";
            mixedascii = false;
        }

        if(multicast) {
            OUTIF() << "Info: Joining multicast address forces UDP and IPv4.";
            udp = true;
            tcp = false;
            ssl = false;
            ipv6 = false;
            ipv4 = true;
            http = false;
            dtls = false;
        }


        if (tcp && udp) {
            OUTIF() << "Warning: both TCP and UDP set. Defaulting to TCP.";
            udp = false;
            dtls = false;
        }
        if (tcp && dtls) {
            OUTIF() << "Warning: both TCP and DTLS set. Defaulting to TCP.";
            udp = false;
            dtls = false;
        }
        if (tcp && ssl) {
            OUTIF() << "Warning: both TCP and SSL set. Defaulting to SSL.";
            tcp = false;
        }

        if (udp && dtls) {
            OUTIF() << "Warning: both UDP and DTLS set. Defaulting to DTLS.";
            udp = false;
        }

        if (http && tcp) {
            OUTIF() << "Warning: both HTTP and TCP set. Defaulting to HTTP.";
            tcp = false;
        }

        if (!filePath.isEmpty() && !QFile::exists(filePath)) {
            OUTIF() << "Error: specified path " << filePath << " does not exist.";
            filePath.clear();
            OUTPUT();
            return -1;
        }
        if (!bindIPstr.isEmpty()) {
            QHostAddress address(bindIPstr);
            if ((QAbstractSocket::IPv4Protocol == address.protocol() ) || (QAbstractSocket::IPv6Protocol == address.protocol())
                    ) {
                OUTIF() << "Binding to custom IP " << bindIPstr;
                bindIP = address;
            } else {
                OUTIF() << "Error: " << bindIPstr << " is an invalid address.";
                OUTPUT();
                return -1;
            }
        }
        if(ipv4 && ipv6) {
            OUTIF() << "Warning: both ipv4 and ipv6 are set. Defaulting to ipv4.";
            ipv6 = false;
        }
        if(!bindIPstr.isEmpty() && ipv4) {
            OUTIF() << "Warning: both ipv4 and custom IP bind are set. Defaulting to custom IP.";
            ipv4 = false;
        }
        if(!bindIPstr.isEmpty() && ipv6) {
            OUTIF() << "Warning: both ipv6 and custom IP bind are set. Defaulting to custom IP.";
            ipv6 = false;
        }
        if(ipv4) {
            QDEBUG() << "bindIP set to IPv4";
            bindIP = QHostAddress(QHostAddress::AnyIPv4);
        }
        if(ipv6) {
            QDEBUG() << "bindIP set to IPv6";
            bindIP = QHostAddress(QHostAddress::AnyIPv6);
        }

        //bind is now default 0

        if (!bind && parser.isSet(bindPortOption)) {
            OUTIF() << "Warning: Binding to port zero is dynamic.";
        }


        if (!port && name.isEmpty() && !http && !server) {
            OUTIF() << "Warning: Sending to port zero.";
        }

        //set default choices
        if (!hex && !ascii && !mixedascii) {
            if(http) {
                mixedascii = true;
            } else {
                hex = true;
            }
        }

        if (!tcp && !udp && !ssl && !http && !dtls) {
            tcp = true;
        }

        if ((tcp || ssl || dtls || http) && intense) {
            OUTIF() << "Warning: Intense Traffic is UDP only.";
        }

        if(intense) {
            udp = true;
            tcp = false;
            ssl = false;
            http = false;
            dtls = false;
        }

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        bool translateMacroSend = settings.value("translateMacroSendCheck", true).toBool();

        if(server) {
            bool bindResult = false;
            MainPacketReceiver * receiver = new MainPacketReceiver(nullptr);
            if(!response.isEmpty()) {
                Packet replyPacket;
                QDEBUGVAR(response);
                replyPacket.hexString = Packet::ASCIITohex(response);
                receiver->responsePacket(replyPacket);
                if(!replyPacket.hexString.isEmpty()) {
                    OUTIF() << "Loading response packet.";

                }
            }
            QString bindIP = "any";
            if(!bindIPstr.isEmpty()) {
                bindIP = bindIPstr;

            }

            QString bindmode = "";
            if(dtls) {
                // TODO: how to set up DTLS server?
            } else {
                if(udp) {
                    bindmode = "UDP";
                    QUdpSocket sock;
                    bindResult = receiver->initUDP(bindIP, bind);
                    bind = receiver->udpSocket->localPort();
                    bindIP = receiver->udpSocket->localAddress().toString();
                } else {
                    if(tcp) {
                        bindmode = "TCP";
                    }
                    if(ssl) {
                        bindmode = "SSL";
                    }
                    bindResult = receiver->initSSL(bindIP, bind, ssl);
                    bind = receiver->tcpServer->serverPort();
                    bindIP = receiver->tcpServer->serverAddress().toString();
                }
            }


            bindIP = bindIP.toUpper();
            if(bindResult) {
                OUTIF() << bindmode << " Server started on " << bindIP << ":" << bind;
                OUTIF() << "Use ctrl+c to exit.";
                OUTPUT();
                a.exec();
            } else {
                OUTIF() << "Failed to bind " << bindmode << " " << bindIP << ":" << bind;
                if(bind < 1024) {
                    OUTIF() << "Note that lower port numbers may require admin/sudo";
                }
                OUTPUT();
            }



            OUTIF() << "Done with server mode. ";

            OUTPUT();
            return 0;
        }



        //Create the packet to send.
        if (!name.isEmpty()) {
            sendPacket = Packet::fetchFromDB(name);

            if(translateMacroSend && (!intense)) {
                QString data = Packet::macroSwap(sendPacket.asciiString());
                sendPacket.hexString = Packet::ASCIITohex(data);
            }


            if (sendPacket.name.isEmpty()) {
                OUTIF() << "Error: Saved packet \"" << name << "\" not found.";
                OUTPUT();
                return -1;
            } else {
                QDEBUGVAR(sendPacket.name);

                ssl = sendPacket.isSSL();
                tcp = sendPacket.isTCP();
                udp = sendPacket.isUDP();
                dtls = sendPacket.isDTLS();
                http = sendPacket.isHTTP() || sendPacket.isHTTPS();

                if (data.isEmpty() && (!http)) {
                    data  = sendPacket.hexString;
                    hex = true;
                    ascii = false;
                    mixedascii = false;
                }
                if (!port) {
                    port  = sendPacket.port;
                }
                if (address.isEmpty()) {
                    address  = sendPacket.toIP;
                }

                if (parser.isSet(udpOption)) {
                    udp = true;
                    ssl = false;
                    tcp = false;
                    dtls = false;
                }
                if (parser.isSet(tcpOption)) {
                    tcp = true;
                    http = false;
                    udp = false;
                    dtls = false;
                }
                if (parser.isSet(sslOption)) {
                    ssl = true;
                    tcp = true;
                    dtls = false;
                    http = false;
                }

                if (intense) {
                    udp = true;
                    ssl = false;
                    tcp = false;
                    http = false;
                    dtls = false;
                }
            }

        }

        if (!parser.isSet(bindPortOption)) {
            bind = 0;
        }

        if (!filePath.isEmpty() && QFile::exists(filePath)) {
            QFile dataFile(filePath);
            if (dataFile.open(QFile::ReadOnly)) {

                if (tcp || ssl || http) {
                    QByteArray dataArray = dataFile.read(1024 * 1024 * 100);;
                    dataString = Packet::byteArrayToHex(dataArray);
                } else {

                    QByteArray dataArray = dataFile.read(1024 * 1024 * 10);
                    dataString = Packet::byteArrayToHex(dataArray);
                }

                dataFile.close();

                //data format is raw.
                ascii = 0;
                hex = 0;
                mixedascii = 0;

            }
        }



        QDEBUGVAR(argssize);
        QDEBUGVAR(quiet);
        QDEBUGVAR(hex);
        QDEBUGVAR(mixedascii);
        QDEBUGVAR(ascii);
        QDEBUGVAR(address);
        QDEBUGVAR(port);
        QDEBUGVAR(wait);
        QDEBUGVAR(bind);
        QDEBUGVAR(bindIP);
        QDEBUGVAR(ipv4);
        QDEBUGVAR(ipv6);
        QDEBUGVAR(tcp);
        QDEBUGVAR(udp);
        QDEBUGVAR(dtls);
        QDEBUGVAR(ssl);
        QDEBUGVAR(http);
        QDEBUGVAR(sslNoError);
        QDEBUGVAR(name);
        QDEBUGVAR(data);
        QDEBUGVAR(filePath);
        QDEBUGVAR(intense);
        QDEBUGVAR(bps);
        QDEBUGVAR(stopnum);
        QDEBUGVAR(rate);
        QDEBUGVAR(usdelay);
        QDEBUGVAR(translateMacroSend);
        QDEBUGVAR(maxrate);
        QDEBUGVAR(server);
        QDEBUGVAR(response);



        //NOW LETS DO THIS!

        if (dtls && !PacketNetwork::DTLSisSupported()) {
            OUTIF() << "Error: This computer does not support DTLS.";
            OUTIF() << "The expected SSL version is " << QSslSocket::sslLibraryBuildVersionString();
            OUTPUT();
            return -1;
        }


        if (ssl && !QSslSocket::supportsSsl()) {
            OUTIF() << "Error: This computer does not have a native SSL library.";
            OUTIF() << "The expected SSL version is " << QSslSocket::sslLibraryBuildVersionString();
            OUTPUT();
            return -1;
        }


        if (ascii) { //pure ascii
            if((!intense)) {
                data = Packet::macroSwap(data);
            }
            dataString = Packet::byteArrayToHex(data.toLatin1());
        }

        if (hex) { //hex
            dataString = Packet::byteArrayToHex(Packet::HEXtoByteArray(data));

            if((!intense)) {
                Packet pkt;
                pkt.hexString = dataString;
                QString data = Packet::macroSwap(pkt.asciiString());
                dataString = Packet::ASCIITohex(data);
            }

        }

        if (mixedascii) { //mixed ascii
            if((!intense)) {
                data = Packet::macroSwap(data);
            }
            dataString = Packet::ASCIITohex(data);
        }

        if (dataString.isEmpty() && !http) {
            OUTIF() << "Warning: No data to send. Is your formatting correct?";
        }


        // HTTP section
        // Test packet 1:  .\packetsender.com --name "HTTPS POST Params"
        // Test packet 2: .\packetsender.com --http GET "https://httpbin.org/get"
        // Test packet 3: .\packetsender.com --http POST "https://httpbin.org/post" "{""hello"":1}"

        if(http) {            
            if ((!address.isEmpty()) && address.contains("://")) {
                sendPacket.requestPath = Packet::getRequestFromURL(address);
                sendPacket.tcpOrUdp = Packet::getMethodFromURL(address);
                sendPacket.port = Packet::getPortFromURL(address);
                sendPacket.toIP = Packet::getHostFromURL(address);
            }

            if(httpMethod.contains("POST")) {
                sendPacket.tcpOrUdp.replace("Get", "Post");
            }

            sendPacket.persistent = false;

            if (!dataString.isEmpty()) {
                sendPacket.hexString = dataString;

            }


            QString url = "";
            QString portUrl = "";
            QString portUrlS = ":" + QString::number(sendPacket.port);

            if(sendPacket.isHTTPS()) {
                url = "https://";
                if(sendPacket.port != 443) {
                    portUrl = portUrlS;
                }
            } else {
                url = "http://";
                if(sendPacket.port != 80) {
                    portUrl = portUrlS;
                }
            }

            QDEBUG() << "URL is" << sendPacket.toIP + portUrl + sendPacket.requestPath;


            QDEBUG() << "HTTP" << sendPacket.tcpOrUdp << sendPacket.requestPath;
            MainPacketReceiver * receiver = new MainPacketReceiver(nullptr);


            OUTIF() << sendPacket.tcpOrUdp <<" " << address << " " << dataString;

            receiver->send(sendPacket);

            for(int i=0; i<10; i++) {
                QCoreApplication::processEvents();
                QThread::msleep(500);
                if(receiver->finished) {
                    break;
                }
            }

            OUTPUT();
            if(!receiver->receivedPacket.hexString.isEmpty()) {
                if (quiet) {
                    out << "\n" << receiver->receivedPacket.asciiString();
                } else {
                    out << "\n" << QString(receiver->receivedPacket.getByteArray());
                }
            }

            return 0;

        }


        QHostAddress addy;
        if (!addy.setAddress(address) && !http) {
            QHostInfo info = QHostInfo::fromName(address);
            if (info.error() != QHostInfo::NoError) {
                OUTIF() << "Error: Could not resolve address:" + address;
                OUTPUT();
                return -1;
            } else {
                addy = info.addresses().at(0);

                if (QAbstractSocket::IPv6Protocol == addy.protocol()) {
                    QDEBUG() << "Valid IPv6 address.";
                    ipv6 = true;
                }

                //domain names are now on-demand connections.
                //address = addy.toString();
            }
        }

        QHostAddress theAddress(address);
        if (QAbstractSocket::IPv6Protocol == theAddress.protocol()) {
            QDEBUG() << "Valid IPv6 address.";
            ipv6 = true;
        }


        QByteArray sendData = sendPacket.HEXtoByteArray(dataString);
        QByteArray recvData;
        recvData.clear();
        int bytesWriten = 0;


        if(dtls) {
            //TODO: How to set up DTLS client?


        }


        if (tcp || ssl) {
            QSslSocket sock;

            bool bindsuccess = sock.bind(bindIP, bind);
            if(!bindsuccess) {
                OUTIF() << "Error: Could not bind to " << bindIP.toString() << ":" << bind;
                OUTPUT();
                return -1;
            }


            if (ssl) {
                sock.connectToHostEncrypted(address,  port);
                if (!sslNoError) {
                    sock.ignoreSslErrors();
                }
            }

            if (tcp && (!ssl)) {
                sock.connectToHost(addy, port);
            }

            sock.waitForConnected(1000);

            QList<QSslError> sslErrorsList;
            sslErrorsList.clear();

            if (ssl) {
                sock.waitForEncrypted(5000);

                QList<QSslError> sslErrorsList  = sock

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                            .sslErrors();
#else
                            .sslHandshakeErrors();
#endif



                if (sslErrorsList.size() > 0) {
                    QSslError sError;
                    foreach (sError, sslErrorsList) {
                        OUTIF() << "SSL Error: " << sError.errorString();
                    }
                }
            }


            if (sock.state() == QAbstractSocket::ConnectedState) {
                QString connectionType = "TCP";
                if (sock.isEncrypted()) {
                    connectionType = "SSL";
                } else {
                    if (ssl) {
                        OUTIF() << "Warning: This connection is not encrypted.";
                    }
                }

                if (sslNoError && (sslErrorsList.size() > 0)) {
                    OUTIF() << "Warning: Abandoning sending data because of SSL no error option.";
                    dataString.clear();
                    sendData.clear();
                }

                QString dataStringTruncated = dataString;
                dataStringTruncated.truncate(100 * 3);
                int chopped = (dataString.size() / 3) - (dataStringTruncated.size() / 3);
                if (chopped > 0) {
                    dataStringTruncated.append("[... ");
                    dataStringTruncated.append(QString::number(chopped));
                    dataStringTruncated.append(" bytes not shown ...]");
                }

                OUTIF() <<  connectionType << " (" << sock.localPort() << ")://" << address << ":" << port << " " << dataStringTruncated;
                if (sock.isEncrypted()) {
                    QSslCipher cipher = sock.sessionCipher();
                    OUTIF() << "Cipher: Encrypted with " << cipher.encryptionMethod();
                }


                bytesWriten = sock.write(sendData);
                sock.waitForBytesWritten(1000);
                //OUTIF() << "Sent:" << Packet::byteArrayToHex(sendData);
                OUTPUT();
                QDeadlineTimer deadlineTimer(wait);
                if(wait <= 0) {
                    deadlineTimer.setDeadline(100);
                }
                while ((!deadlineTimer.hasExpired()) && (sock.state() == QAbstractSocket::ConnectedState)) {
                    sock.waitForReadyRead(100);
                    recvData = sock.readAll();
                    if (recvData.isEmpty()) {
                        continue;
                    }
                    QString hexString = Packet::byteArrayToHex(recvData);
                    if (quiet) {
                        out << "\n" << hexString;

                    } else {
                        out << "\nResponse Time:" << QDateTime::currentDateTime().toString(DATETIMEFORMAT);
                        out << "\nResponse HEX:" << hexString;
                        out << "\nResponse ASCII:" << Packet::hexToASCII(hexString);
                    }

                    out.flush();

                }
                sock.disconnectFromHost();
                if (sock.state() != QAbstractSocket::UnconnectedState) {
                    sock.waitForDisconnected(1000);
                }
                sock.close();

                OUTPUT();
                return 0;


            } else {
                OUTIF() << "Error: Failed to connect to " << address;

                OUTPUT();
                return -1;
            }


        } else {
            QUdpSocket sock;
            sock.setSocketOption(QAbstractSocket::MulticastTtlOption, 128);

            bool bindsuccess = sock.bind(bindIP, bind);
            if(!bindsuccess) {
                OUTIF() << "Error: Could not bind to " << bindIP.toString() << ":" << bind;
                OUTPUT();
                return -1;
            }


            if(multicast) {
                bool didjoin = sock.joinMulticastGroup(QHostAddress(address));
                if(!didjoin) {
                    OUTIF() << "Error: Could not join multicast group " << address;
                    OUTIF() << "Attempting to send anyway...";
                }

            }



            OUTIF() << "UDP (" << sock.localPort() << ")://" << address << ":" << port << " " << dataString;

            if(intense) {
                if(translateMacroSend) {
                    OUTIF() << "Warning: Macros can slow down intense traffic";
                }
                OUTIF() << "Starting Intense Traffic Generator";
                OUTPUT();

                if(maxrate) {
                    OUTIF() << "Max rate traffic generator will use separate sockets per thread";


// Threaded traffic generator requires at least Qt 5.10 (which the AppImage does not use yet)
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                    QUdpSocket sock;
                    QTextStream out(stdout);
                    intenseTrafficGenerator(out, sock, addy, port, dataString, bps, rate, stopnum, usdelay);
#else

                    QList<QThread *> threadedTraffic;
                    int max_threads = QThread::idealThreadCount();
                    for(int thread_i = 0; thread_i < max_threads; thread_i++ ) {


                        QThread *thread = QThread::create([addy, port, dataString, bps, rate, stopnum, usdelay]{
                            QUdpSocket sock;
                            QTextStream out(stdout);
                            intenseTrafficGenerator(out, sock, addy, port, dataString, bps, rate, stopnum, usdelay);
                        });

                        threadedTraffic.append(thread);
                    }


                    int threadcount = 0;
                    foreach(QThread *thread, threadedTraffic) {
                        out << "Starting thread" << threadcount;
                        thread->start();
                    }

                    while(!threadedTraffic.isEmpty()) {
                        QThread::sleep(1);
                    }


#endif

                } else {
                    int done = intenseTrafficGenerator(out, sock, addy, port, dataString, bps, rate, stopnum, usdelay);
                    return done;

                }


            }


            bytesWriten = sock.writeDatagram(sendData, addy, port);
            //OUTIF() << "Wrote " << bytesWriten << " bytes";
            sock.waitForBytesWritten(1000);


            OUTPUT();
            QDeadlineTimer deadlineTimer(wait);
            if(wait <= 0) {
                deadlineTimer.setDeadline(100);
            }
            while (!deadlineTimer.hasExpired()) {
                sock.waitForReadyRead(100);

                if (sock.hasPendingDatagrams()) {

                    out << MainPacketReceiver::datagramOutput(sock.receiveDatagram(10000000), quiet);
                    out.flush();
                }
            }

            if(multicast) {
                sock.leaveMulticastGroup(QHostAddress(address));
            }

            sock.close();

            OUTPUT();
            return 0;

        }





        OUTPUT();



    } else {



#ifdef __linux__
#ifndef ISSNAP

        //Workaround linux check for those that support xrandr. Does not work for snaps
        //Note that this bug is actually within Qt.
        if(!force_gui) {
            if (!isGuiApp()) {
                printf("\nCannot open display. Try --help to access console app. Try --gui to bypass this check.\n");
                return -1;
            }
        } else {
            QDEBUG() << "Bypassing GUI check";
        }
#endif
#endif


#ifndef CONSOLE_BUILD


        QApplication a(argc, argv);

        QDEBUGVAR(args);

        qRegisterMetaType<Packet>();

        QTranslator translator, translator_qt, translator_qtbase;

        // Locale translation...
        QString locale = QLocale::system().name().section("", 0, 2);
        QDEBUGVAR(locale);


        if(Settings::needLanguage()) {
            LanguageChooser lang;
            lang.exec();
        }


        QString language = Settings::language();


        if(language == "Chinese") {
            QDEBUG() << "qt lang loaded" << translator_qt.load(QString("qt_zh_cn"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "base lang loaded" << translator_qtbase.load(QString("qtbase_zh_cn"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "app lang loaded" << translator.load(":/languages/packetsender_cn.qm");
            QDEBUG() << QApplication::installTranslator(&translator_qt) << QApplication::installTranslator(&translator_qtbase) << QApplication::installTranslator(&translator) ;
        }

        if(language == "Spanish") {
            QDEBUG() << "qt lang loaded" << translator_qt.load(QString("qt_es"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "base lang loaded" << translator_qtbase.load(QString("qtbase_es"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "app lang loaded" << translator.load(":/languages/packetsender_es.qm");
            QDEBUG() << QApplication::installTranslator(&translator_qt) << QApplication::installTranslator(&translator_qtbase) << QApplication::installTranslator(&translator) ;
        }

        if(language == "German") {
            QDEBUG() << "qt lang loaded" << translator_qt.load(QString("qt_de"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "base lang loaded" << translator_qtbase.load(QString("qtbase_de"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "app lang loaded" << translator.load(":/languages/packetsender_de.qm");
            QDEBUG() << QApplication::installTranslator(&translator_qt) << QApplication::installTranslator(&translator_qtbase) << QApplication::installTranslator(&translator) ;
        }

        if(language == "French") {
            QDEBUG() << "qt lang loaded" << translator_qt.load(QString("qt_fr"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "base lang loaded" << translator_qtbase.load(QString("qtbase_fr"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "app lang loaded" << translator.load(":/languages/packetsender_fr.qm");
            QDEBUG() << QApplication::installTranslator(&translator_qt) << QApplication::installTranslator(&translator_qtbase) << QApplication::installTranslator(&translator) ;
        }


        if(language == "Italian") {
            QDEBUG() << "qt lang loaded" << translator_qt.load(QString("qt_it"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "base lang loaded" << translator_qtbase.load(QString("qtbase_it"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            QDEBUG() << "app lang loaded" << translator.load(":/languages/packetsender_it.qm");
            QDEBUG() << QApplication::installTranslator(&translator_qt) << QApplication::installTranslator(&translator_qtbase) << QApplication::installTranslator(&translator) ;
        }


        QFile file_system(":/packetsender.css");
        QFile file_dark(":/qdarkstyle/style.qss");

        //Use default OS styling for non-Windows. Too many theme variants.

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        bool useDark = Settings::useDark();
        QString styleSheet = "";

        if(useDark) {
            if (file_dark.open(QFile::ReadOnly)) {
                styleSheet = QLatin1String(file_dark.readAll());
                a.setStyleSheet(styleSheet);
                file_dark.close();
            }
        } else {
            if (file_system.open(QFile::ReadOnly)) {
                styleSheet = QLatin1String(file_system.readAll());
                a.setStyleSheet(styleSheet);
                file_system.close();
            }
        }


        //panels_only = true;
        if(panels_only) {

            QSettings settings(SETTINGSFILE, QSettings::IniFormat);

            bool darkMode = Settings::useDark();
            PanelGenerator::darkMode = darkMode;
            PanelGenerator::renderOnly = panels_only;


            PacketNetwork packetNetwork;
            packetNetwork.init();
            PanelGenerator * gpanel = new PanelGenerator();
            if(!styleSheet.isEmpty()) {
                gpanel->setStyleSheet(styleSheet);
            }

            QDEBUG() << " packet send connect attempt:" << QObject::connect(gpanel, SIGNAL(sendPacket(Packet)),
                     &packetNetwork, SLOT(packetToSend(Packet)));

            gpanel->initAutoLaunchOrEditMode();
            gpanel->show();
            return a.exec();

        } else {

            MainWindow w;
            w.show();
            return a.exec();

        }



#endif

    }


    return 0;
}



int intenseTrafficGenerator(QTextStream &out, QUdpSocket &sock, QHostAddress addy, unsigned int port, QString hexString, double bps, double rate, qint64 stopnum, qint64 usdelay)
{

    QSettings settings(SETTINGSFILE, QSettings::IniFormat);
    bool translateMacroSend = settings.value("translateMacroSendCheck", true).toBool();

    QByteArray sendData = Packet::HEXtoByteArray(hexString);

    if(bps > 0.1 && usdelay == 0) {
        QDEBUG() << "Convert bps to rate for bytes :"  << sendData.size();
        double bytespersecond = bps / 8;
        double totalbytes = (sendData.size() + 20);
        rate = bytespersecond  / totalbytes;
        out << "Calculated rate to send a " << totalbytes << " byte UDP packet at " << bps << " bps is " << rate << " packets/second\n";
    }

    auto hasstop = stopnum > 0;

    if(hasstop) {
        if(stopnum < 50) {
            out << "Rate calculation is unreliable with low stop number. \n";
        }
    }

    if(rate > 0 && rate < 0.2) {
        out << "Slowest supported rate is 0.2. Exiting.\n";
        return -1;
    }

    if(bps > 0 && bps < 0.2) {
        out << "Slowest supported bps is 0.2. Exiting.\n";
        return -1;
    }

    qint64 stopcounter = 0;
    if(hasstop) {
        out << "Will stop sending after " << stopnum << " packets\n";
    }

    Packet pkt;
    pkt.hexString = Packet::byteArrayToHex(sendData);


    QElapsedTimer totalTime;
    totalTime.start();

    if(rate < 0.2 && usdelay == 0 && bps < 0.1) {
        out << "Sending as fast as possible. Use Ctrl+C to quit.\n";
        while(1) {

            if(translateMacroSend) {
                QString data = Packet::macroSwap(pkt.asciiString());
                pkt.hexString = Packet::ASCIITohex(data);
                sendData = pkt.getByteArray();
            }

            sock.writeDatagram(sendData, addy, port);
            STOPSENDCHECK();

        }
    } else {

        // Calculate the msdelay.
        double msdelay = (1000 / rate);

        if(rate > 0.2 && usdelay == 0) {

            out << "Sending at a rate of " << rate << " packets/second\n";
            QDEBUGVAR(rate);
            QDEBUGVAR(msdelay);
            QElapsedTimer elasped;
            elasped.start();
            while(1) {

                if(translateMacroSend) {
                    QString data = Packet::macroSwap(pkt.asciiString());
                    pkt.hexString = Packet::ASCIITohex(data);
                    sendData = pkt.getByteArray();
                }

                sock.writeDatagram(sendData, addy, port);
                STOPSENDCHECK();

                while(1) {
                    qint64 t = elasped.elapsed();
                    if(t > msdelay) {
                        elasped.start();
                        break;
                    } else {
                        double telapsed_diff = msdelay - t * 1.0;
                        if (telapsed_diff > 5) {
                            QThread::msleep(5);
                        }
                    }

                }
            }
            QDEBUG();


        } else {
            QDEBUG();

            out << "Sending using usdelay set to " << usdelay << "\n";
            while(1) {
                sock.writeDatagram(sendData, addy, port);
                STOPSENDCHECK();

                QThread::usleep(usdelay);
                QCoreApplication::processEvents();

            }

        }
    }

    auto elasped = totalTime.elapsed();
    out << "Run time: " << elasped <<" milliseconds\n";
    if(stopcounter > 0) {
        qint64 totalbytes = stopcounter * (sendData.size() + 20);
        double elaspedDouble = static_cast<double>(elasped);
        double stopcounterDouble = static_cast<double>(stopcounter);
        double totalbytesDouble = static_cast<double>(totalbytes);
        double effectiverate = (totalbytesDouble) * 8 * 1000  / (elaspedDouble);
        double effectivehertz = (stopcounterDouble) * 1000 / (elaspedDouble );
        out << "Sent " << stopcounter <<" packets, " << totalbytes << " total bytes \n";
        out << "Effective rate = " << effectivehertz <<" packets per second\n";
        out << "Effective bps = " << effectiverate <<" bits per second\n";
    }

    return 0;
}



