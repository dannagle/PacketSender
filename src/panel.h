#pragma once

#include <QList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>


#include "packet.h"

struct PanelButton {
    int id;
    QString title;
    QString script;
};



class Panel
{
public:
    Panel();
    int id;
    QString name;
    int launch;
    QString lastmodfied;
    QStringList linkTexts;
    QStringList linkURLs;
    void init(QList<Packet> packets);
    void copy(Panel other);
    QList<PanelButton> buttonList;
    static Panel fromDB(QString thename);
    static Panel fromDB(int theid);
    static Panel getLaunchPanel();
    static int newPanelID(QList<Panel> allPanels);
    static QList<Panel> fetchAllPanels();
    static QByteArray ExportJSON(QList<Panel> panelList);
    static QList<Panel> ImportJSON(QByteArray data);
    void saveToDB();
    void clear();
    QString toString();
    static void sortByID(QList<Panel> &panelList);
    bool isLaunchPanel();
    void deleteFromDB();
    QDateTime getLastModified();
    void sortButtons();
};

