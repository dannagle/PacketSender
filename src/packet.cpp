/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright NagleCode, LLC
 *
 */

#include "packet.h"

#include <QDebug>
#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QPair>
#include <QDesktopServices>
#include <QUuid>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <time.h>



const int Packet::PACKET_NAME = Qt::UserRole +  0;
const int Packet::PACKET_HEX = Qt::UserRole + 1;
const int Packet::FROM_IP = Qt::UserRole +     2;
const int Packet::FROM_PORT = Qt::UserRole +       3;
const int Packet::TO_PORT = Qt::UserRole + 4;
const int Packet::TO_IP = Qt::UserRole +     5;

const int Packet::TIMESTAMP = Qt::UserRole + 7;
const int Packet::DATATYPE = Qt::UserRole + 8;
const int Packet::TCP_UDP = Qt::UserRole + 9;
const int Packet::REPEAT = Qt::UserRole + 10;
const int Packet::INCOMING = Qt::UserRole + 11;
const int Packet::REQUEST_URL = Qt::UserRole + 12;



//macro to get value from DB
#define FROMDB_UINT(a) packet.a = settings.value(nameFound + "/"+ # a).toUInt()
#define FROMDB_FLOAT(a) packet.a = settings.value(nameFound + "/"+ # a).toFloat()
#define FROMDB_ULONG(a) packet.a = settings.value(nameFound + "/"+ # a).toULongLong()
#define FROMDB_STRING(a) packet.a = settings.value(nameFound + "/" + # a).toString()

//save to DB macro
#define TODB(a) settings.setValue(name + "/"+ # a, a)

void Packet::clear()
{
    init();
}

bool Packet::isSSL()
{
    return (tcpOrUdp.trimmed().toLower().contains("ssl"));
}

bool Packet::isUDP()
{
    return ((tcpOrUdp.trimmed().toLower() == "udp"));
}

bool Packet::isHTTP()
{
    return ((tcpOrUdp.trimmed().toLower().contains("http")));
}
bool Packet::isHTTPS()
{
    return ((tcpOrUdp.trimmed().toLower().contains("https")));
}
bool Packet::isPOST()
{
    return  isHTTP() && ((tcpOrUdp.trimmed().toLower().contains("post")));
}

bool Packet::isTCP()
{
    return ((tcpOrUdp.trimmed().toLower().contains("tcp") || isSSL()));
}

float Packet::oneDecimal(float value)
{
    float valueFloat = value * 10;
    int valueInt = (int) valueFloat;
    valueFloat = ((float) valueInt) / 10;
    return valueFloat;
}


Packet::~Packet()
{
    init();
}

#define OTHEREQUALS(var) var = other.var
Packet::Packet(const Packet &other)
{
    OTHEREQUALS(name);
    OTHEREQUALS(hexString);
    OTHEREQUALS(fromIP);
    OTHEREQUALS(toIP);
    OTHEREQUALS(errorString);
    OTHEREQUALS(repeat);
    OTHEREQUALS(port);
    OTHEREQUALS(fromPort);
    OTHEREQUALS(tcpOrUdp);
    OTHEREQUALS(sendResponse);
    OTHEREQUALS(timestamp);
    OTHEREQUALS(receiveBeforeSend);
    OTHEREQUALS(delayAfterConnect);
    OTHEREQUALS(persistent);
    OTHEREQUALS(incoming);
    OTHEREQUALS(requestPath);
}

QHostAddress Packet::IPV4_IPV6_ANY(QString ipMode)
{
    QHostAddress h4 = QHostAddress("0.0.0.0");
    QHostAddress h6 = QHostAddress("::");

    if(ipMode == "4") {
        return h4;
    }

    if(ipMode == "6") {
        return h6;
    }


    QHostAddress address(ipMode);

    if ((QAbstractSocket::IPv4Protocol == address.protocol() ) || (QAbstractSocket::IPv6Protocol == address.protocol())
            ) {
        return address;
    }

    return h4;
}

void Packet::init()
{
    name = "";
    hexString = "";
    fromIP = "";
    toIP = "";
    errorString = "";
    port = 55005;
    tcpOrUdp = "TCP";
    sendResponse = 0;
    repeat = 0;
    incoming = false;
    timestamp = QDateTime::currentDateTime();
    receiveBeforeSend = false;
    delayAfterConnect = 0;
    persistent = false;
}


#define JSONSTR(VAR) json[QString(# VAR).toLower()] = packetList[i].VAR
#define JSONNUM(VAR) json[QString(# VAR).toLower()] = QString::number(packetList[i].VAR)

QByteArray Packet::ExportJSON(QList<Packet> packetList)
{
    QByteArray returnData;

    QJsonArray jsonArray;

    for (int i = 0; i < packetList.size(); i++) {


        QJsonObject json;
        if (packetList[i].name.isEmpty()) {
            continue;
        }
        json["name"] = packetList[i].name;
        JSONSTR(hexString);
        JSONSTR(fromIP);
        JSONSTR(toIP);
        JSONSTR(errorString);
        JSONNUM(port);
        JSONNUM(fromPort);
        JSONSTR(tcpOrUdp);
        JSONNUM(sendResponse);
        JSONSTR(requestPath);
        JSONSTR(repeat);
        json["asciistring"] = QString(packetList[i].asciiString().toLatin1().toBase64());
        //JSONSTR(timestamp);

        jsonArray.push_front(json);
    }

    QJsonDocument doc(jsonArray);

    returnData = doc.toJson();


    return returnData;
}

QList<Packet> Packet::ImportJSON(QByteArray data)
{
    QList<Packet> returnList;

    QJsonDocument doc = QJsonDocument::fromJson(data);


    if (!doc.isNull()) {
        //valid json
        if (doc.isArray()) {
            //valid array
            QJsonArray jsonArray = doc.array();
            if (!jsonArray.isEmpty()) {
                QDEBUG() << "Found" <<  jsonArray.size() << "packets";

                for (int i = 0; i < jsonArray.size(); i++) {
                    Packet pkt;
                    pkt.clear();
                    QJsonObject json = jsonArray[i].toObject();

                    pkt.name = json["name"].toString();
                    pkt.errorString = json["errorstring"].toString();
                    pkt.fromIP = json["fromip"].toString();
                    pkt.fromPort = json["fromport"].toString().toUInt();
                    pkt.hexString = json["hexstring"].toString();
                    if(json.contains("requestpath")) {
                        pkt.requestPath = json["requestpath"].toString();
                    }
                    pkt.toIP = json["toip"].toString();
                    pkt.port = json["port"].toString().toUInt();
                    pkt.repeat = json["repeat"].toString().toFloat();
                    pkt.sendResponse = json["sendresponse"].toString().toUInt();
                    pkt.tcpOrUdp = json["tcporudp"].toString();

                    returnList.append(pkt);

                }


            }

        }
    }


    return returnList;
}


SendPacketButton * Packet::getSendButton(QTableWidget * parent)
{
    SendPacketButton * returnButton = new SendPacketButton(parent);
    returnButton->name = name;
    returnButton->init();
    returnButton->setText("Send");
    returnButton->setToolTip("Send <b>" + name + "</b>");
    returnButton->setProperty("name", name);
    fromIP = "YOU";
    returnButton->setIcon(getIcon());

    return returnButton;

}

QIcon Packet::getIcon()
{
    if (isHTTP()) {
        if (fromIP.toUpper().contains("YOU")) {
            QIcon myIcon(HTTPSENDICON);
            return myIcon;
        } else {
            QIcon myIcon(HTTPRXICON);
            return myIcon;
        }
    }
    if (isUDP()) {
        if (fromIP.toUpper().contains("YOU")) {
            QIcon myIcon(UDPSENDICON);
            return myIcon;
        } else {
            QIcon myIcon(UDPRXICON);
            return myIcon;
        }

    }


    if (isTCP()) {

        if (fromIP.toUpper().contains("YOU")) {
            QIcon myIcon(TCPSENDICON);
            return myIcon;
        } else {
            QIcon myIcon(TCPRXICON);
            return myIcon;
        }

    }


    if (isSSL()) {

        if (fromIP.toUpper().contains("YOU")) {
            QIcon myIcon(SSLSENDICON);
            return myIcon;
        } else {
            QIcon myIcon(SSLRXICON);
            return myIcon;
        }

    }

    //I don't know what it is...
    QIcon myIcon(TCPRXICON);
    return myIcon;

}

QString Packet::hexToASCII(QString &hex)
{


    QStringList hexSplit;

    //remove invalid characters of popular deliminators...
    hex = hex.replace(",", " ");
    hex = hex.replace(".", " ");
    hex = hex.replace(":", " ");
    hex = hex.replace(";", " ");
    hex = hex.replace("0x", " ");
    hex = hex.replace("x", " ");
    hex = hex.replace("\n", " ");
    hex = hex.replace("\r", " ");
    hex = hex.replace("\t", " ");

    QString hexText = hex.simplified();
    if (hexText.isEmpty()) {
        return "";
    }

    if ((hexText.size() % 2 != 0)) {
        //Not divisible by 2. What should I do?
        if (!hexText.contains(" ") && hexText.size() > 2) {
            //Seems to be one big hex stream. Front-load it with a 0.
            hexText.prepend("0");
        }

    }


    if (!hexText.contains(" ") && hexText.size() > 2 && hexText.size() % 2 == 0) {
        //does not contain any spaces.  Maybe one big hex stream?
        QDEBUG() << "no spaces" << "even digits";
        QStringList hexList;
        hexList.clear();
        QString append;
        append.clear();
        for (int i = 0; i < hexText.size(); i += 2) {
            append.clear();
            append.append(hexText[i]);
            append.append(hexText[i + 1]);
            hexList << append;
        }
        hexText = hexList.join(" ").trimmed();
        hex = hexText;
    }

    hexSplit = hexText.split(" ");
    QString asciiText = "";
    unsigned int convertInt;
    bool ok = false;
    int malformed = 0;
    bool malformedBool = false;
    QChar malformedChar;


    QString checkSpace = hex.at(hex.size() - 1);
    if (checkSpace == " ") {
        hexText.append(" ");
    }

    hex = hexText;

    // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ <<"analyze hex split" << hexSplit;

    for (int i = 0; i < hexSplit.size(); i++) {
        if (hexSplit.at(i).size() > 2) {
            malformedBool = true;
            malformed = i;
            malformedChar = hexSplit.at(i).at(2);
            // qDebug() << __FILE__ << "/" << __LINE__ << __FUNCTION__  << "malformed at"<< QString::number(i) << "is" << malformedChar;
            break;
        }

    }

    if (malformedBool) {
        QString fixText = "";
        QString testChar;

        for (int i = 0; i < malformed; i++) {
            fixText.append(hexSplit.at(i));
            fixText.append(" ");
        }


        testChar.append(malformedChar);
        testChar.toUInt(&ok, 16);

        // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ << "malformed digitvalue" << malformedChar.digitValue();

        if (ok) {
            fixText.append(hexSplit.at(malformed).at(0));
            fixText.append(hexSplit.at(malformed).at(1));
            fixText.append(" ");
            fixText.append(malformedChar);
        }
        hexText = (fixText.simplified());
        hex = hexText;
        hexSplit = hexText.split(" ");
    }



    for (int i = 0; i < hexSplit.size(); i++) {
        convertInt = hexSplit.at(i).toUInt(&ok, 16);
        // qDebug() << __FILE__ << "/" << __LINE__ << __FUNCTION__  <<"hex at"<< QString::number(i) << "is" << QString::number(convertInt);
        if (ok) {
            if (convertInt >= 0x20 && convertInt <= 0x7e && convertInt != '\\') {
                // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ << "Converted to " << QChar(convertInt);
                asciiText.append((QChar(convertInt)));
            } else {
                asciiText.append("\\");
                switch ((char)convertInt) {
                    case '\n':
                        asciiText.append("n");
                        break;
                    case '\r':
                        asciiText.append("r");
                        break;
                    case '\t':
                        asciiText.append("t");
                        break;
                    case '\\':
                        asciiText.append("\\");
                        break;
                    default:
                        if (convertInt < 16) {
                            asciiText.append("0");
                        }
                        asciiText.append(QString::number(convertInt, 16));
                        break;

                }

            }

        } else {
            // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ << "Convert failed";
            hexSplit[i] = "";
            hex = (hexSplit.join(" "));
        }

    }


    return asciiText;

}

QString Packet::byteArrayToHex(QByteArray data)
{
    QString byte, returnString, returnStringTemp;
    //  QDEBUG() << "size is " <<data.size();

    if (data.isEmpty()) {
        return "";
    }

    int datasize = data.size();

    returnStringTemp = data.toHex().toUpper();
    returnString.resize(datasize * 3, ' ');

    int j = 0;
    for (int i = 0; i < returnStringTemp.size(); i += 2) {
        returnString[j] = returnStringTemp[i];
        returnString[j + 1] = returnStringTemp[i + 1];
        j += 3;
    }

    return returnString;

}

int Packet::hexToInt(QChar hex)
{
    hex = hex.toLower();

    if (hex == 'f') {
        return 15;
    }
    if (hex == 'e') {
        return 14;
    }
    if (hex == 'd') {
        return 13;
    }
    if (hex == 'c') {
        return 12;
    }
    if (hex == 'b') {
        return 11;
    }
    if (hex == 'a') {
        return 10;
    }

    return hex.digitValue();

}

QByteArray Packet::getByteArray()
{
    return HEXtoByteArray(hexString);
}

QString Packet::asciiString()
{
    QString hex = hexString;
    QString ascii = Packet::hexToASCII(hex);
    return ascii;
}


void Packet::saveToDB()
{

    QList<QString> packets;
    packets.clear();
    bool foundName = false;

    QSettings settings(PACKETSFILE, QSettings::IniFormat);

    int size = settings.beginReadArray(NAMEINIKEY);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString namekey = settings.value("name").toString();
        if (namekey == name) {
            foundName = true;
        }
        packets.append(namekey);
    }
    settings.endArray();

    if (!foundName) {
        packets.append(name);
        settings.beginWriteArray(NAMEINIKEY);
        for (int i = 0; i < packets.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("name", packets.at(i));
        }
        settings.endArray();
    }

    //Save variables to DB using macro
    //All packets
    TODB(name);
    TODB(fromIP);
    TODB(repeat);
    TODB(toIP);
    TODB(port);
    TODB(fromPort);
    TODB(tcpOrUdp);
    TODB(sendResponse);
    TODB(hexString);
    TODB(requestPath);
    settings.setValue(name + "/timestamp", timestamp.toString("ddd, d MMM yyyy hh:mm:ss"));


}


Packet Packet::fetchFromList(QString thename, QList<Packet> packets)
{
    Packet returnPacket, packet;
    returnPacket.init();

    foreach (packet, packets) {

        if (packet.name == thename) {
            return packet;
        }
    }

    //return empty packet if not found
    return returnPacket;
}


Packet Packet::fetchFromDB(QString thename)
{
    QList<Packet> packets =  Packet::fetchAllfromDB("");
    return Packet::fetchFromList(thename, packets);
 }


bool comparePacketsByName(const Packet &packetA, const Packet &packetB)
{
    return  packetA.name.toLower() < packetB.name.toLower();
}

bool comparePacketsByTime(const Packet &packetA, const Packet &packetB)
{
    if (packetA.timestamp == packetB.timestamp) {
        return packetA.toIP < packetB.toIP;
    } else {
        return  packetA.timestamp > packetB.timestamp;
    }
}


void Packet::sortByName(QList<Packet> &packetList)
{

    std::sort(packetList.begin(), packetList.end(), comparePacketsByName);

}
void Packet::sortByTime(QList<Packet> &packetList)
{
    std::sort(packetList.begin(), packetList.end(), comparePacketsByTime);


}


QList<Packet> Packet::fetchAllfromDB(QString importFile)
{
    QList<Packet> packets;
    Packet packet;
    QList<QString> nameList;
    QString nameFound;
    nameList.clear();
    packets.clear();

    if (importFile.isEmpty()) {
        importFile = PACKETSFILE;
    }

    QSettings settings(importFile, QSettings::IniFormat);



    int size = settings.beginReadArray(NAMEINIKEY);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        nameList.append(settings.value("name").toString());
    }
    settings.endArray();


    foreach (nameFound, nameList) {

        //qDebug() << "found mac" << nameFound;

        packet.init();

        FROMDB_STRING(name);
        FROMDB_STRING(toIP);
        FROMDB_UINT(port);
        FROMDB_FLOAT(repeat);
        FROMDB_UINT(fromPort);
        FROMDB_STRING(tcpOrUdp);
        FROMDB_STRING(hexString);
        FROMDB_STRING(requestPath);
        packets.append(packet);
    }

    Packet::sortByName(packets);

    return packets;


}


void Packet::removeFromDBList(QStringList nameList)
{
    QSettings settings(PACKETSFILE, QSettings::IniFormat);
    QList<Packet> packets = Packet::fetchAllfromDB("");


    QDEBUGVAR(nameList.size());

    QList<Packet> packetSaved;
    packetSaved.clear();


    for (int i = 0; i < packets.size(); i++) {
        QString thename = packets[i].name.trimmed();
        if (nameList.contains(thename)) {
            settings.beginGroup(thename);
            settings.remove("");
            settings.endGroup();
        } else {
            packetSaved.append(packets[i]);
        }
    }

    settings.beginWriteArray(NAMEINIKEY);
    for (int i = 0; i < packetSaved.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", packetSaved[i].name);
    }
    settings.endArray();

}


bool Packet::removeFromDB(QString thename)
{
    QSettings settings(PACKETSFILE, QSettings::IniFormat);
    QList<Packet> packets = Packet::fetchAllfromDB("");



    for (int i = 0; i < packets.size(); i++) {
        if (packets[i].name.trimmed() == thename.trimmed()) {
            packets.removeAt(i);
            settings.beginGroup(thename);
            settings.remove("");
            settings.endGroup();
        }
    }

    settings.beginWriteArray(NAMEINIKEY);
    for (int i = 0; i < packets.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", packets[i].name);
    }
    settings.endArray();


    return true;
}


Packet Packet::fetchTableWidgetItemData(QTableWidgetItem * tItem)
{
    Packet returnPacket;
    returnPacket.init();
    returnPacket.name = tItem->data(Packet::PACKET_NAME).toString();
    returnPacket.hexString = tItem->data(Packet::PACKET_HEX).toString();
    returnPacket.toIP = tItem->data(Packet::TO_IP).toString();
    returnPacket.port = tItem->data(Packet::TO_PORT).toUInt();
    returnPacket.tcpOrUdp = tItem->data(Packet::TCP_UDP).toString();
    returnPacket.fromPort = tItem->data(Packet::FROM_PORT).toUInt();
    returnPacket.fromIP = tItem->data(Packet::FROM_IP).toString();
    returnPacket.repeat = tItem->data(Packet::REPEAT).toFloat();
    returnPacket.incoming = tItem->data(Packet::INCOMING).toBool();
    returnPacket.requestPath = tItem->data(Packet::REQUEST_URL).toString();
    return returnPacket;
}

SmartResponseConfig Packet::fetchSmartConfig(int num, QString importFile)
{
    QSettings settings(importFile, QSettings::IniFormat);

    SmartResponseConfig smart;
    smart.id = num;
    smart.encoding = settings.value("responseEncodingBox" + QString::number(num), "").toString();
    smart.ifEquals = settings.value("responseIfEdit" + QString::number(num), "").toString();
    smart.replyWith = settings.value("responseReplyEdit" + QString::number(num), "").toString();
    smart.enabled = settings.value("responseEnableCheck" + QString::number(num), false).toBool();

    return smart;
}

QString Packet::macroSwap(QString data)
{

    QDateTime now = QDateTime::currentDateTime();

    if (data.contains("{{TIME}}")) {
        data = data.replace("{{TIME}}", now.toString("h:mm:ss ap"));
    }
    if (data.contains("{{DATE}}")) {
        data = data.replace("{{DATE}}", now.toString("yyyy-MM-dd"));
    }
    if (data.contains("{{RANDOM}}")) {
	srand(time(NULL));
        data = data.replace("{{RANDOM}}", QString::number(rand()));
    }
    if (data.contains("{{UNIXTIME}}")) {
        data = data.replace("{{UNIXTIME}}", QString::number(now.toMSecsSinceEpoch() / 1000));
    }
    if (data.contains("{{UNIQUE}}")) {
        data = data.replace("{{UNIQUE}}", QUuid::createUuid().toString());
    }

    return data;

}

QByteArray Packet::encodingToByteArray(QString encoding, QString data)
{

    encoding = encoding.trimmed().toLower();

    data = Packet::macroSwap(data);

    if (encoding == "ascii") {
        return data.toLatin1();
    }

    if (encoding == "hex") {
        return Packet::HEXtoByteArray(data);
    }

    //fallback mixed ascii
    QString hex = Packet::ASCIITohex(data);
    return (Packet::HEXtoByteArray(hex));

}

QByteArray Packet::smartResponseMatch(QList<SmartResponseConfig> smartList, QByteArray data)
{
    SmartResponseConfig config;

    QDEBUG() << "Checking smart " << smartList.size() << "For" << Packet::byteArrayToHex(data);

    //the incoming data has already been encoded.

    foreach (config, smartList) {
        if (config.enabled) {
            QByteArray testData = Packet::encodingToByteArray(config.encoding, config.ifEquals);
            if (testData == (data)) {
                QDEBUG() << "Match! Sending:" << config.replyWith;
                return Packet::encodingToByteArray(config.encoding, config.replyWith);
            }
        }
    }

    QByteArray noData;
    noData.clear();
    return noData;
}

bool Packet::operator()(const Packet *a, const Packet *b) const
{
    return a->timestamp < b->timestamp;
}


void Packet::populateTableWidgetItem(QTableWidgetItem * tItem, Packet thepacket)
{
    tItem->setData(Packet::PACKET_NAME, thepacket.name);
    tItem->setData(Packet::PACKET_HEX,  thepacket.hexString);
    tItem->setData(Packet::FROM_IP,  thepacket.fromIP);
    tItem->setData(Packet::TO_IP,  thepacket.toIP);
    tItem->setData(Packet::TO_PORT,  thepacket.port);
    tItem->setData(Packet::FROM_PORT,  thepacket.fromPort);
    tItem->setData(Packet::TCP_UDP,  thepacket.tcpOrUdp);
    tItem->setData(Packet::REPEAT,  thepacket.repeat);
    tItem->setData(Packet::INCOMING,  thepacket.repeat);
    tItem->setData(Packet::REQUEST_URL,  thepacket.requestPath);
    QByteArray thedata = thepacket.getByteArray();
    tItem->setToolTip("Data portion is " + QString::number(thedata.size()) + " bytes");
}



void Packet::setBoldItem(QTableWidgetItem * tItem, Packet thepacket)
{
    Q_UNUSED(tItem);
    Q_UNUSED(thepacket);
    //TODO:This does not work well.
    //The packet itself should know if it is was received or sent.
    /*
    QFont originalFont = (tItem)->font();
    if (!thepacket.fromIP.toUpper().contains("YOU")) {
        originalFont.setBold(true);
        tItem->setFont(originalFont);
    } else {
        originalFont.setBold(false);
    }
    */
}


QByteArray Packet::HEXtoByteArray(QString thehex)
{

    //function already ignores invalid chars...
    return QByteArray::fromHex(thehex.toLatin1());;
}

QString Packet::removeIPv6Mapping(QHostAddress ipv6)
{
    quint32 ipv4 = ipv6.toIPv4Address();

    //valid address will have a result greater than 0
    if (ipv4 > 0) {
        QHostAddress new_ipv4(ipv4);
        return new_ipv4.toString();
    } else {
        return ipv6.toString();
    }

}

QString Packet::ASCIITohex(QString &ascii)
{
    if (ascii.isEmpty()) {
        return "";
    }

    QString asciiText = ascii;
    QString hexText = "";
    QChar charTest;
    QString convertTest;
    bool msb = false;
    bool lsb = false;
    int lsbInt = 0;
    int msbInt = 0;

    // qDebug() << __FILE__ << "/" << __LINE__;

    //convert special sequences to raw numbers.
    asciiText.replace("\\\\", "\\" + QString::number('\\', 16));
    asciiText.replace("\\n", "\\0" + QString::number('\n', 16));
    asciiText.replace("\\r", "\\0" + QString::number('\r', 16));
    asciiText.replace("\\t", "\\0" + QString::number('\t', 16));

    // qDebug() << __FILE__ << "/" << __LINE__;
    if (asciiText.size() > 0) {
        if (asciiText.at(asciiText.size() - 1) == '\\') { //last char is a slash
            asciiText.append("00");
        }
    }

    // qDebug() << __FILE__ << "/" << __LINE__;
    if (asciiText.size() > 2) {
        if (asciiText.at(asciiText.size() - 2) == '\\') { //second last char is a slash
            //slide 0 in between

            // qDebug() << __FILE__ << "/" << __LINE__ <<"second last is slash";

            charTest = asciiText.at(asciiText.size() - 1);
            asciiText[asciiText.size() - 1] = '0';
            asciiText.append(charTest);
        }
    }
    // qDebug() << __FILE__ << "/" << __LINE__ <<"analyze" << asciiText;


    for (int i = 0 ; i < asciiText.size(); i++) {
        msb = false;
        lsb = false;
        lsbInt = 0;
        msbInt = 0;

        charTest = asciiText.at(i);

        // qDebug() << __FILE__ << "/" << __LINE__ <<"checking" << charTest;

        if (charTest == '\\') {
            // qDebug() << __FILE__ << "/" << __LINE__ <<"found slash";
            if (i + 1 < asciiText.size()) {
                msbInt = hexToInt(asciiText.at(i + 1));
                if (msbInt > -1) {
                    msb = true;
                }
                // qDebug() << __FILE__ << "/" << __LINE__ <<"msb convert test is" << msb;

            }
            if (i + 2 < asciiText.size()) {
                lsbInt = hexToInt(asciiText.at(i + 2));
                if (lsbInt > -1) {
                    lsb = true;
                }
                // qDebug() << __FILE__ << "/" << __LINE__ <<"lsb convert test is" << lsb;
            }

            if (msb) {
                hexText.append(QString::number(msbInt, 16));
                // qDebug() << __FILE__ << "/" << __LINE__ <<"hexText append result" << hexText;
                i++;
            }

            if (lsb) {
                hexText.append(QString::number(lsbInt, 16));
                // qDebug() << __FILE__ << "/" << __LINE__ <<"hexText append" << hexText;
                i++;
            }

        } else {
            // qDebug() << __FILE__ << "/" << __LINE__ <<"no slash";
            lsbInt = ((int) charTest.toLatin1()) & 0xff;
            if (lsbInt > 0 && lsbInt < 16) {
                hexText.append("0");
            }
            hexText.append(QString::number(lsbInt, 16));
            // qDebug() << __FILE__ << "/" << __LINE__ <<"appended lsbInt:" << QString::number(lsbInt, 16);
        }

        hexText.append(" ");
        // qDebug() << __FILE__ << "/" << __LINE__ <<"hex test now " << hexText;

    }

    return hexText;

}

