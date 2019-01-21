#include "packetlogmodel.h"

#include <QClipboard>
#include <QApplication>

PacketLogModel::PacketLogModel(QObject *parent)
    : QAbstractTableModel(parent)
{

    packetList.clear();
    tableHeaders.clear();
}

QVariant PacketLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            return tableHeaders.at(section);
        }
    }
    return QVariant();
}

int PacketLogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    //QDEBUGVAR(packetList.size());

    return packetList.size();

}

int PacketLogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return tableHeaders.size();

}

QVariant PacketLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();


    int row = index.row();
    Packet packet = packetList.at(row);

    if (role == Qt::DecorationRole) {
        if (index.column() == 0) {

            return packet.getIcon();

        }
        return QVariant();
    }

    if (role == Qt::UserRole)
    {
        return row;
    }


    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
    {
        if(index.column() == tableHeaders.indexOf("Time")) {
            return packet.timestamp.toString(DATETIMEFORMAT);
        }
        if(index.column() == tableHeaders.indexOf("From IP")) {
            return packet.fromIP;
        }
        if(index.column() == tableHeaders.indexOf("From Port")) {
            return packet.fromPort;
        }
        if(index.column() == tableHeaders.indexOf("To IP")) {
            return packet.toIP;
        }
        if(index.column() == tableHeaders.indexOf("To Port")) {
            return packet.port;
        }
        if(index.column() == tableHeaders.indexOf("Method")) {
            return packet.tcpOrUdp;
        }
        if(index.column() == tableHeaders.indexOf("Error")) {
            return packet.errorString;
        }
        if(index.column() == tableHeaders.indexOf("ASCII")) {
            return packet.asciiString();
        }
        if(index.column() == tableHeaders.indexOf("Hex")) {
            return packet.hexString;
        }

    }

    return QVariant();

}

bool PacketLogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)
    {
        //do nothing.
    }
    return true;

}

Qt::ItemFlags PacketLogModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}


const Packet PacketLogModel::getPacket(QModelIndex index)
{

    int row = index.row();
    if(packetList.size() >= row) {
        return packetList.at(row);
    }

    const Packet pkt;
    return  pkt;

}

int PacketLogModel::size()
{
    return packetList.size();

}

const Packet PacketLogModel::getPacket(int index)
{
    if(packetList.size() >= index) {
        return packetList.at(index);
    }

    const Packet pkt;
    return  pkt;

}

void PacketLogModel::setTableHeaders(QStringList headers)
{
    beginResetModel();
    tableHeaders = headers;
    endResetModel();
}

void PacketLogModel::removeFirst()
{
    packetList.removeFirst();
}

const QList<Packet> PacketLogModel::list()
{
    const QList<Packet> retList = packetList;

    return retList;
}

void PacketLogModel::clear()
{
    beginResetModel();
    packetList.clear();
    endResetModel();
}

void PacketLogModel::prepend(Packet packet)
{
    beginResetModel();
    packetList.prepend(packet);
    endResetModel();

    QDEBUGVAR(packetList.size());
}

void PacketLogModel::append(Packet packet)
{
    beginResetModel();
    packetList.append(packet);
    endResetModel();

    QDEBUGVAR(packetList.size());
}

void PacketLogModel::setPacketData(QList<Packet> packets)
{
    beginResetModel();
    packetList.clear();
    packetList.append(packets);
    endResetModel();
}
