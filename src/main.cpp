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
    #include "settings.h"
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

        // A single option with a value
        QCommandLineOption httpOption(QStringList() << "http", "Send HTTP. Allowed values are GET (default) and POST", "http");
        parser.addOption(httpOption);

        // An option with a value
        QCommandLineOption nameOption(QStringList() << "n" << "name",
                                      "Send previously saved packet named <name>. Other options overrides saved packet parameters.",
                                      "name");
        parser.addOption(nameOption);



        // Intense Traffic Generator
        QCommandLineOption bpsOption(QStringList() << "bps", "Intense traffic. Calculate rate based on value of bits per second.", "bps");
        parser.addOption(bpsOption);
        QCommandLineOption numOption(QStringList() << "num", "Intense traffic. Number of packets to send. Default unlimited.", "number");
        parser.addOption(numOption);
        QCommandLineOption rateOption(QStringList() << "rate", "Intense traffic. Rate. Ignored in bps option.", "Hertz");
        parser.addOption(rateOption);
        QCommandLineOption usdelayOption(QStringList() << "usdelay", "Intense traffic. Resend delay. Used if rate is 0. Ignored in bps option.", "microseconds");
        parser.addOption(usdelayOption);



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
        bool ssl = parser.isSet(sslOption);
        bool sslNoError = parser.isSet(sslNoErrorOption);
        bool ipv6  = parser.isSet(bindIPv6Option);
        bool ipv4  = parser.isSet(bindIPv4Option);
        bool http  = parser.isSet(httpOption);

        bool okbps = false;
        bool okrate = false;
        bool intense = parser.isSet(bpsOption) || parser.isSet(numOption)|| parser.isSet(rateOption) || parser.isSet(usdelayOption);
        double bps = parser.value(bpsOption).toDouble(&okbps);
        qint64 stopnum = parser.value(numOption).toULongLong();
        double rate = parser.value(rateOption).toDouble(&okrate);
        qint64 usdelay = parser.value(usdelayOption).toULongLong();
        if(intense) {
            if (!okrate && !okbps) {
                OUTIF() << "Warning: Invalid rate and/or bps. Intense traffic will free-run.";
                bps = 0;
                rate = 0;
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
        }


        if (tcp && udp) {
            OUTIF() << "Warning: both TCP and UDP set. Defaulting to TCP.";
            udp = false;
        }
        if (tcp && ssl) {
            OUTIF() << "Warning: both TCP and SSL set. Defaulting to SSL.";
            tcp = false;
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


        if (!port && name.isEmpty() && !http) {
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

        if (!tcp && !udp && !ssl && !http) {
            tcp = true;
        }

        if ((tcp || ssl || http) && intense) {
            OUTIF() << "Warning: Intense Traffic is UDP only.";
        }

        if(intense) {
            udp = true;
            tcp = false;
            ssl = false;
            http = false;
        }

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        bool translateMacroSend = settings.value("translateMacroSendCheck", true).toBool();


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
                }
                if (parser.isSet(tcpOption)) {
                    tcp = true;
                    http = false;
                }
                if (parser.isSet(sslOption)) {
                    ssl = true;
                    tcp = true;
                    http = false;
                }

                if (intense) {
                    udp = true;
                    ssl = false;
                    tcp = false;
                    http = false;
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


        //NOW LETS DO THIS!

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
                return bytesWriten;


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

                int done = intenseTrafficGenerator(out, sock, addy, port, dataString, bps, rate, stopnum, usdelay);
                return done;

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


                    QHostAddress sender;
                    int senderPort;

                    QNetworkDatagram theDatagram = sock.receiveDatagram(10000000);
                    QByteArray recvData = theDatagram.data();
                    sender =  theDatagram.senderAddress();
                    senderPort = theDatagram.senderPort();

                    QString hexString = Packet::byteArrayToHex(recvData);
                    if (quiet) {
                        out << "\n" << hexString;
                    } else {
                        out << "\nFrom: " << sender.toString() << ", Port:" << senderPort;
                        out << "\nResponse Time:" << QDateTime::currentDateTime().toString(DATETIMEFORMAT);
                        out << "\nResponse HEX:" << hexString;
                        out << "\nResponse ASCII:" << Packet::hexToASCII(hexString);
                    }

                    out.flush();
                }
            }

            if(multicast) {
                sock.leaveMulticastGroup(QHostAddress(address));
            }

            sock.close();

            OUTPUT();
            return bytesWriten;

        }





        OUTPUT();



    } else {



#ifdef __linux__
#ifndef ISSNAP

        //Workaround linux check for those that support xrandr. Does not work for snaps
        //Note that this bug is actually within Qt.
        if (!isGuiApp()) {
            printf("\nCannot open display. Try --help to access console app.\n");
            return -1;
        }
#endif
#endif


#ifndef CONSOLE_BUILD


        QApplication a(argc, argv);

        QDEBUGVAR(args);

        qRegisterMetaType<Packet>();

        QTranslator translator;

        /*
        bool loaded = translator.load("D:/github/PacketSender/src/languages/packetsender_es.qm");
        auto done = QApplication::installTranslator(&translator);
        if(loaded && done) {
            QDEBUG() << "Spanish Translation Loaded" << done;
        } else {
            return -1;
        }
        */

        QString language = Settings::language();
        if(language == "Spanish") {
            bool loaded = translator.load(":/languages/packetsender_es.qm");
            if(!loaded) {
                QDEBUG() << "Could not load translation";
            } else {
                auto done = QApplication::installTranslator(&translator);
                QDEBUG() << "Spanish Translation Loaded" << done;
            }
        } else {
            QDEBUG() << "Loading Default English";
        }

        QFile file_system(":/packetsender.css");
        QFile file_dark(":/qdarkstyle/style.qss");

        //Use default OS styling for non-Windows. Too many theme variants.

        QSettings settings(SETTINGSFILE, QSettings::IniFormat);
        bool useDark = settings.value("darkModeCheck", true).toBool();

        if(useDark) {
            if (file_dark.open(QFile::ReadOnly)) {
                QString StyleSheet = QLatin1String(file_dark.readAll());
                a.setStyleSheet(StyleSheet);
                file_dark.close();
            }
        } else {
            if (file_system.open(QFile::ReadOnly)) {
                QString StyleSheet = QLatin1String(file_system.readAll());
                a.setStyleSheet(StyleSheet);
                file_system.close();
            }
        }


        MainWindow w;


        w.show();

        return a.exec();

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



