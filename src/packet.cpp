/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */

#include "packet.h"

#include <QDebug>
#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QPair>
#include <QDesktopServices>



const int Packet::PACKET_NAME = Qt::UserRole +  0;
const int Packet::PACKET_HEX = Qt::UserRole + 1;
const int Packet::FROM_IP = Qt::UserRole +     2;
const int Packet::FROM_PORT = Qt::UserRole +       3;
const int Packet::TO_PORT = Qt::UserRole +4;
const int Packet::TO_IP = Qt::UserRole +     5;
const int Packet::SEND_RESPONSE = Qt::UserRole +     6;
const int Packet::TIMESTAMP = Qt::UserRole + 7;
const int Packet::DATATYPE = Qt::UserRole + 8;
const int Packet::TCP_UDP = Qt::UserRole + 9;
const int Packet::REPEAT = Qt::UserRole + 10;

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

bool Packet::isTCP()
{
   return (tcpOrUdp.trimmed().toLower() == "tcp");
}

float Packet::oneDecimal(float value) {
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
    timestamp = QDateTime::currentDateTime();
    receiveBeforeSend = false;
    delayAfterConnect = 0;
    persistent = false;
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
    if(tcpOrUdp.toUpper() == "UDP")
    {
        if(fromIP.toUpper().contains("YOU"))
        {
            QIcon myIcon(UDPSENDICON);
            return myIcon;
        } else {
            QIcon myIcon(UDPRXICON);
            return myIcon;
        }

    } else {

        if(fromIP.toUpper().contains("YOU"))
        {
            QIcon myIcon(TCPSENDICON);
            return myIcon;
        } else {
            QIcon myIcon(TCPRXICON);
            return myIcon;
        }

    }

}

QString Packet::hexToASCII(QString &hex)
{


    QStringList hexSplit;
    QString hexText = hex.simplified();
    if(hexText.isEmpty())
    {
        return "";
    }

    if(!hexText.contains(" ") && hexText.size() > 2 && hexText.size() % 2 == 0)
    {
        //does not contain any spaces.  Maybe one big hex stream?
        QDEBUG() << "no spaces" << "even digits";
        QStringList hexList;
        hexList.clear();
        QString append;
        append.clear();
        for(int i =0; i < hexText.size(); i+=2)
        {
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
    if(checkSpace == " ")
    {
        hexText.append(" ");
    }

    hex = hexText;

    // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ <<"analyze hex split" << hexSplit;

    for(int i=0; i< hexSplit.size(); i++)
    {
        if(hexSplit.at(i).size() > 2)
        {
            malformedBool = true;
            malformed = i;
            malformedChar = hexSplit.at(i).at(2);
            // qDebug() << __FILE__ << "/" << __LINE__ << __FUNCTION__  << "malformed at"<< QString::number(i) << "is" << malformedChar;
            break;
        }

    }

    if(malformedBool)
    {
        QString fixText = "";
        QString testChar;

        for(int i = 0; i < malformed; i++)
        {
            fixText.append(hexSplit.at(i));
            fixText.append(" ");
        }


        testChar.append(malformedChar);
        testChar.toUInt(&ok, 16);

        // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ << "malformed digitvalue" << malformedChar.digitValue();

        if(ok)
        {
            fixText.append(hexSplit.at(malformed).at(0));
            fixText.append(hexSplit.at(malformed).at(1));
            fixText.append(" ");
            fixText.append(malformedChar);
        }
        hexText = (fixText.simplified());
        hex = hexText;
        hexSplit = hexText.split(" ");
    }



    for(int i=0; i< hexSplit.size(); i++)
    {
        convertInt = hexSplit.at(i).toUInt(&ok, 16);
        // qDebug() << __FILE__ << "/" << __LINE__ << __FUNCTION__  <<"hex at"<< QString::number(i) << "is" << QString::number(convertInt);
        if(ok)
        {
            if(convertInt >= 0x20 && convertInt < 0x7e && convertInt != '\\')
            {
                // qDebug() << __FILE__ << "/" << __LINE__  << __FUNCTION__ << "Converted to " << QChar(convertInt);
                asciiText.append((QChar(convertInt)));
            } else {
                asciiText.append("\\");
                switch((char)convertInt)
                {
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
                    if(convertInt < 16)
                    {
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
    QString byte, returnString;
  //  QDEBUG() << "size is " <<data.size();

    returnString.clear();
    if(data.isEmpty())
    {
        return "";
    }

    for(int i = 0; i < data.size(); i++)
    {
        byte = QString::number((unsigned char)data.at(i) & 0xff, 16);

        if(byte.size() % 2 == 1)
        {
            byte.prepend("0");
        }
        returnString.append(byte);
        returnString.append(" ");
    }

    return returnString.trimmed().toUpper();

}

int Packet::hexToInt(QChar hex)
{
    hex = hex.toLower();

    if(hex == 'f')
    {
        return 15;
    }
    if(hex == 'e')
    {
        return 14;
    }
    if(hex == 'd')
    {
        return 13;
    }
    if(hex == 'c')
    {
        return 12;
    }
    if(hex == 'b')
    {
        return 11;
    }
    if(hex == 'a')
    {
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
        if(namekey == name)
        {
            foundName = true;
        }
        packets.append(namekey);
    }
    settings.endArray();

    if(!foundName)
    {
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
    settings.setValue(name + "/timestamp", timestamp.toString("ddd, d MMM yyyy hh:mm:ss"));


}


Packet Packet::fetchFromDB(QString thename)
{
    QList<Packet> packets =  Packet::fetchAllfromDB("");
    Packet returnPacket, packet;
    returnPacket.init();

    foreach(packet, packets)
    {

        if(packet.name == thename)
        {
            return packet;
        }
    }

    //return empty packet if not found
    return returnPacket;

}


bool comparePacketsByName(const Packet &packetA,const Packet &packetB)
{
    return  packetA.name.toLower() < packetB.name.toLower();
}

bool comparePacketsByTime(const Packet &packetA,const Packet &packetB)
{
    if(packetA.timestamp == packetB.timestamp) {
        return packetA.toIP < packetB.toIP;
    } else {
        return  packetA.timestamp > packetB.timestamp;
    }
}


void Packet::sortByName(QList<Packet> &packetList)
{

    qSort(packetList.begin(), packetList.end(), comparePacketsByName);

}
void Packet::sortByTime(QList<Packet> &packetList)
{
    qSort(packetList.begin(), packetList.end(), comparePacketsByTime);


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


    foreach (nameFound, nameList)
    {

        //qDebug() << "found mac" << nameFound;

        packet.init();

        FROMDB_STRING(name);
        FROMDB_STRING(toIP);
        FROMDB_UINT(port);
        FROMDB_FLOAT(repeat);
        FROMDB_UINT(fromPort);
        FROMDB_STRING(tcpOrUdp);
        FROMDB_STRING(hexString);
        packets.append(packet);
    }

    Packet::sortByName(packets);

    return packets;


}

bool Packet::removeFromDB(QString thename)
{
    QSettings settings(PACKETSFILE, QSettings::IniFormat);
    QList<Packet> packets = Packet::fetchAllfromDB("");



     for(int i = 0; i < packets.size(); i++)
     {
         if(packets[i].name.trimmed() == thename.trimmed())
         {
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
    returnPacket.fromPort= tItem->data(Packet::FROM_PORT).toUInt();
    returnPacket.fromIP = tItem->data(Packet::FROM_IP).toString();
    returnPacket.repeat = tItem->data(Packet::REPEAT).toFloat();
    return returnPacket;
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
    QByteArray thedata = thepacket.getByteArray();
    tItem->setToolTip("Data portion is " + QString::number(thedata.size()) + " bytes");
}

QByteArray Packet::HEXtoByteArray(QString thehex)
{
     QString byte;
     QByteArray returnArray;
     QStringList hexSplit = thehex.simplified().split(" ");
     returnArray.clear();
     unsigned int foundByte = 0;
     bool ok =  false;

     foreach(byte, hexSplit)
     {
         foundByte = byte.toUInt(&ok, 16);
         foundByte = foundByte & 0xff;
         if(ok)
         {
             returnArray.append(foundByte);
         }

     }

     return returnArray;

}

QString Packet::removeIPv6Mapping(QHostAddress ipv6)
{
        return ipv6.toString();

}

QString Packet::ASCIITohex(QString &ascii)
{
    if(ascii.isEmpty())
    {
        return "";
    }

    QString asciiText = ascii;
    QString hexText = "";
    QChar tempChar1, tempChar2;
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
    if(asciiText.size() > 0)
    {
        if(asciiText.at(asciiText.size()-1) == '\\') //last char is a slash
        {
            asciiText.append("00");
        }
    }

    // qDebug() << __FILE__ << "/" << __LINE__;
    if(asciiText.size() > 2)
    {
        if(asciiText.at(asciiText.size()-2) == '\\') //second last char is a slash
        {
            //slide 0 in between

            // qDebug() << __FILE__ << "/" << __LINE__ <<"second last is slash";

            charTest = asciiText.at(asciiText.size()-1);
            asciiText[asciiText.size()-1] = '0';
            asciiText.append(charTest);
        }
    }
    // qDebug() << __FILE__ << "/" << __LINE__ <<"analyze" << asciiText;


    for (int i = 0 ; i < asciiText.size(); i++)
    {
        msb = false;
        lsb = false;
        lsbInt = 0;
        msbInt = 0;

        charTest = asciiText.at(i);

        // qDebug() << __FILE__ << "/" << __LINE__ <<"checking" << charTest;

        if(charTest == '\\')
        {
            // qDebug() << __FILE__ << "/" << __LINE__ <<"found slash";
            if(i + 1 < asciiText.size())
            {
                msbInt = hexToInt(asciiText.at(i + 1));
                if(msbInt > -1)
                {
                    msb = true;
                }
                // qDebug() << __FILE__ << "/" << __LINE__ <<"msb convert test is" << msb;

            }
            if(i + 2 < asciiText.size())
            {
                lsbInt = hexToInt(asciiText.at(i + 2));
                if(lsbInt > -1)
                {
                    lsb = true;
                }
                // qDebug() << __FILE__ << "/" << __LINE__ <<"lsb convert test is" << lsb;
            }

            if(msb)
            {
                hexText.append(QString::number(msbInt, 16));
                // qDebug() << __FILE__ << "/" << __LINE__ <<"hexText append result" << hexText;
                i++;
            }

            if(lsb)
            {
                hexText.append(QString::number(lsbInt, 16));
                // qDebug() << __FILE__ << "/" << __LINE__ <<"hexText append" << hexText;
                i++;
            }

        } else {
            // qDebug() << __FILE__ << "/" << __LINE__ <<"no slash";
            lsbInt = ((int) charTest.toLatin1()) & 0xff;
            if(lsbInt > 0 && lsbInt < 16)
            {
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
