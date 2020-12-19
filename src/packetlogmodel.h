#ifndef PACKETLOGMODEL_H
#define PACKETLOGMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "packet.h"


class PacketLogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PacketLogModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override ;


    void setTableHeaders(QStringList headers);

    void setPacketData(QList<Packet> packets);
    void append(Packet packet);
    const Packet getPacket(int index);
    const Packet getPacket(QModelIndex index);
    int size();
    void clear();
    void removeFirst();

    const QList<Packet> list();

    bool useEllipsis;


    void prepend(Packet packet);
private:

    QList<Packet> packetList;
    QStringList tableHeaders;

};

#endif // PACKETLOGMODEL_H
