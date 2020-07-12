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
#include <QDeadlineTimer>
#include <QProcess>

#include "mainwindow.h"
#define DEBUGMODE 0


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

        QCoreApplication::setApplicationName("Packet Sender");
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

        // A boolean option with a single name (-p)
        QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Quiet mode. Only output received data.");
        parser.addOption(quietOption);


        QCommandLineOption hexOption(QStringList() << "x" << "hex", "Parse data-to-send as hex (default).");
        parser.addOption(hexOption);

        QCommandLineOption asciiOption(QStringList() << "a" << "ascii", "Parse data-to-send as mixed-ascii (like the GUI).");
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

        // An option with a value
        QCommandLineOption nameOption(QStringList() << "n" << "name",
                                      "Send previously saved packet named <name>. Other options overrides saved packet parameters.",
                                      "name");
        parser.addOption(nameOption);


        parser.addPositionalArgument("address", "Destination address. Optional for saved packet.");
        parser.addPositionalArgument("port", "Destination port. Optional for saved packet.");
        parser.addPositionalArgument("data", "Data to send. Optional for saved packet.");


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

        if (sslNoError) ssl = true;

        QString name = parser.value(nameOption);

        QString filePath = parser.value(fileOption);


        QString address = "";
        QString addressOriginal = "";
        unsigned int port = 0;

        int argssize = args.size();
        QString data, dataString;
        data.clear();
        dataString.clear();
        if (argssize >= 1) {
            address = args[0];
        }
        if (argssize >= 2) {
            port = args[1].toUInt();
        }
        if (argssize >= 3) {
            data = (args[2]);
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
        }


        if (tcp && udp) {
            OUTIF() << "Warning: both TCP and UDP set. Defaulting to TCP.";
            udp = false;
        }
        if (tcp && ssl) {
            OUTIF() << "Warning: both TCP and SSL set. Defaulting to SSL.";
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


        if (!port && name.isEmpty()) {
            OUTIF() << "Warning: Sending to port zero.";
        }

        //set default choices
        if (!hex && !ascii && !mixedascii) {
            hex = true;
        }

        if (!tcp && !udp && !ssl) {
            tcp = true;
        }



        //Create the packet to send.
        if (!name.isEmpty()) {
            sendPacket = Packet::fetchFromDB(name);
            if (sendPacket.name.isEmpty()) {
                OUTIF() << "Error: Saved packet \"" << name << "\" not found.";
                OUTPUT();
                return -1;
            } else {

                ssl = sendPacket.isSSL();
                tcp = sendPacket.isTCP();
                udp = sendPacket.isUDP();

                if (data.isEmpty()) {
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
                }
                if (parser.isSet(sslOption)) {
                    ssl = true;
                    tcp = true;
                }
            }

        }

        if (!parser.isSet(bindPortOption)) {
            bind = 0;
        }

        if (!filePath.isEmpty() && QFile::exists(filePath)) {
            QFile dataFile(filePath);
            if (dataFile.open(QFile::ReadOnly)) {

                if (tcp || ssl) {
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
        QDEBUGVAR(sslNoError);
        QDEBUGVAR(name);
        QDEBUGVAR(data);
        QDEBUGVAR(filePath);

        //NOW LETS DO THIS!

        if (ssl && !QSslSocket::supportsSsl()) {
            OUTIF() << "Error: This computer does not have a native SSL library.";
            OUTIF() << "The expected SSL version is " << QSslSocket::sslLibraryBuildVersionString();
            OUTPUT();
            return -1;
        }


        if (ascii) { //pure ascii
            dataString = Packet::byteArrayToHex(data.toLatin1());
        }

        if (hex) { //hex
            dataString = Packet::byteArrayToHex(Packet::HEXtoByteArray(data));
        }

        if (mixedascii) { //mixed ascii
            dataString = Packet::ASCIITohex(data);
        }

        if (dataString.isEmpty()) {
            OUTIF() << "Warning: No data to send. Is your formatting correct?";
        }

        QHostAddress addy;
        if (!addy.setAddress(address)) {
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

                QList<QSslError> sslErrorsList  = sock.sslErrors();


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
        //Workaround linux check for those that support xrandr
        //Note that this bug is actually within Qt.
        if (!isGuiApp()) {
            printf("\nCannot open display. Try --help to access console app.\n");
            return -1;
        }
#endif

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


        MainWindow w;


        w.show();

        return a.exec();

    }



    return 0;

}
