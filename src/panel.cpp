#include "panel.h"


#include "globals.h"
#include <QDebug>
#include <QStandardPaths>
#include <algorithm>
#include <QHash>
#include <QPair>
#include <QFile>

Panel::Panel()
{
    clear();
}

bool comparePanelsByID(const Panel &panelA, const Panel &panelB);
bool comparePanelButtonsByID(const PanelButton &panelA, const PanelButton &panelB);



#define PANELJSONSTR(VAR) json[QString(# VAR).toLower()] = panelList[i].VAR
#define PANELJSONNUM(VAR) json[QString(# VAR).toLower()] = QString::number(panelList[i].VAR)

#define BUTTONJSONSTR(VAR) jsonButton[QString(# VAR).toLower()] = panelList[i].buttonList[j].VAR
#define BUTTONJSONNUM(VAR) jsonButton[QString(# VAR).toLower()] = QString::number(panelList[i].buttonList[j].VAR)

QByteArray Panel::ExportJSON(QList<Panel> panelList)
{
    QByteArray returnData;

    QJsonArray jsonArray;

    for (int i = 0; i < panelList.size(); i++) {
        QDEBUGVAR(panelList[i].buttonList.size());
        QDEBUGVAR(panelList[i].linkTexts.size());
        QDEBUGVAR(panelList[i].linkURLs.size());

        QJsonObject json;
        if (panelList[i].name.isEmpty()) {
            continue;
        }
        PANELJSONSTR(name);
        PANELJSONNUM(id);
        PANELJSONNUM(launch);
        PANELJSONSTR(lastmodfied);

        //iterate the link texts
        QJsonArray linkURLsArray;
        for (int j = 0; j < panelList[i].linkTexts.size(); j++) {
            QJsonObject linkButton;
            if(j >= panelList[i].linkURLs.size()) {
                QDEBUG() << "Problem!" << j;
                break;
            }
            linkButton["text"] = panelList[i].linkTexts[j];
            linkButton["url"] = panelList[i].linkURLs[j];
            linkButton["index"] = QString::number(j);

            linkURLsArray.push_front(linkButton);
        }

        json["linkURLs"] = linkURLsArray;
        QDEBUGVAR(linkURLsArray.size());

        //panelList[i].


        //iterate the panel buttons
        QJsonArray buttonArray;
        for (int j = 0; j < panelList[i].buttonList.size(); j++) {
            QJsonObject jsonButton;
            if (panelList[i].buttonList[j].title.isEmpty()) {
                QDEBUG() << "Missing title on buttonList. skipping";
                continue;
            }

            BUTTONJSONSTR(title);
            BUTTONJSONNUM(id);
            BUTTONJSONSTR(script);

            buttonArray.push_front(jsonButton);

        }

        json["buttonlist"] = buttonArray;

        jsonArray.push_front(json);
    }

    QJsonDocument doc(jsonArray);

    returnData = doc.toJson();


    return returnData;
}

void Panel::sortButtons()
{

    std::sort(buttonList.begin(), buttonList.end(), comparePanelButtonsByID);
}

bool comparePanelsByID(const Panel &panelA, const Panel &panelB)
{
    return  panelA.id < panelB.id;
}

bool comparePanelButtonsByID(const PanelButton &panelA, const PanelButton &panelB)
{
    return  panelA.id < panelB.id;
}


void Panel::sortByID(QList<Panel> &panelList)
{
    std::sort(panelList.begin(), panelList.end(), comparePanelsByID);
}


void Panel::copy(Panel other)
{
    this->name = other.name;
    this->id = other.id;
    this->launch = other.launch;
    this->lastmodfied = other.lastmodfied;

    this->buttonList.clear();
    this->linkTexts.clear();
    this->linkURLs.clear();

    this->buttonList.append(other.buttonList);
    this->linkTexts.append(other.linkTexts);
    this->linkURLs.append(other.linkURLs);
}


QString Panel::toString()
{
    QString text;
    QTextStream out(&text);
    out << "id:" << id << ",name:" <<name<<",lastmodified:" << lastmodfied <<",launch:"
        << launch    <<", buttonList:" << buttonList.size()<<", linkURLs:" << linkURLs.size();
    return text;
}

QList<Panel> Panel::ImportJSON(QByteArray data)
{
    QList<Panel> returnList;

    QJsonDocument doc = QJsonDocument::fromJson(data);


    if (!doc.isNull()) {
        //valid json
        if (doc.isArray()) {
            //valid array
            QJsonArray jsonArray = doc.array();
            if (!jsonArray.isEmpty()) {
                QDEBUG() << "Found" <<  jsonArray.size() << "panel";

                for (int i = 0; i < jsonArray.size(); i++) {
                    Panel panel;
                    panel.clear();
                    QJsonObject json = jsonArray[i].toObject();

                    panel.name = json["name"].toString();
                    panel.lastmodfied = json["lastmodfied"].toString();
                    QString idcheck = json["id"].toString();
                    panel.id = idcheck.toInt();

                    idcheck = json["launch"].toString();
                    panel.launch = idcheck.toInt();

                    if(json["buttonlist"].isArray()) {
                        QJsonArray buttonListArray = json["buttonlist"].toArray();
                        for (int j = 0; j < buttonListArray.size(); j++) {

                            QJsonObject jsonButton = buttonListArray[j].toObject();

                            PanelButton panelButton;
                            panelButton.title = jsonButton["title"].toString();
                            panelButton.script = jsonButton["script"].toString();
                            idcheck = jsonButton["id"].toString();
                            panelButton.id = idcheck.toInt();


                            panel.buttonList.append(panelButton);

                        }
                    }


                    QHash<int, QPair<QString,QString>> urlsList;
                    if(json["linkURLs"].isArray()) {
                        QJsonArray linkURLsArray = json["linkURLs"].toArray();
                        for (int j = 0; j < linkURLsArray.size(); j++) {

                            QJsonObject jsonURL = linkURLsArray[j].toObject();
                            QString indexStr = jsonURL["index"].toString();
                            QString urlStr = jsonURL["url"].toString();
                            QString textStr = jsonURL["text"].toString();
                            int index = indexStr.toInt();
                            urlsList[index].first = textStr;
                            urlsList[index].second = urlStr;
                        }

                        auto intList = urlsList.keys();
                        panel.linkTexts.clear();
                        panel.linkURLs.clear();
                        for (int j = 0; j < intList.size(); j++) {
                            panel.linkTexts.append(urlsList[intList[j]].first);
                            panel.linkURLs.append(urlsList[intList[j]].second);
                        }
                    }


                    panel.sortButtons();

                    QDEBUGVAR(panel.toString());

                    returnList.append(panel);

                }
            }
        }
    }

    Panel::sortByID(returnList);
    return returnList;
}


void Panel::clear()
{
    buttonList.clear();
    name.clear();
    id = 0;
    launch = 0;
    lastmodfied.clear();
}

QList<Panel> Panel::fetchAllPanels()
{
    QList<Panel> panels;

    QFile saveFile(PANELSFILE);

    if(saveFile.open(QFile::ReadOnly)) {
        auto saveData = saveFile.readAll();
        saveFile.close();
        panels = ImportJSON(saveData);
    }


    return panels;

}

bool Panel::isLaunchPanel()
{
    return launch > 0;
}

QDateTime Panel::getLastModified()
{
    QDateTime lm;
    lm.fromString(lastmodfied, FULLDATETIMEFORMAT);
    return lm;
}


void Panel::saveToDB()
{
    auto allPanels = fetchAllPanels();
    bool found = false;

    lastmodfied  = QDateTime::currentDateTime().toString(FULLDATETIMEFORMAT);

    Panel pb;
    pb.id = id;
    pb.name = name;
    pb.buttonList.append(buttonList);
    pb.linkTexts.append(linkTexts);
    pb.linkURLs.append(linkURLs);
    pb.launch = launch;
    pb.lastmodfied = lastmodfied;
    QDEBUGVAR(pb.buttonList.size());
    for (int i=0; i<allPanels.size(); i++) {
        if(allPanels[i].id == id) {
            found = true;
            allPanels[i].copy(pb);
            break;
        }
        if(this->isLaunchPanel()) {
            //turn off launch panel on all others.
            if(allPanels[i].id != id) {
                allPanels[i].launch = 0;
            }
        }
    }

    if(!found) {
        QDEBUG() << "Did not find panel";
        allPanels.append(pb);
    }

    QDEBUGVAR(allPanels.size());

    QByteArray saveData = Panel::ExportJSON(allPanels);

    QFile saveFile(PANELSFILE);

    if(saveFile.open(QFile::WriteOnly)) {
        saveFile.write(saveData);
        saveFile.close();
    }

}

void Panel::deleteFromDB()
{
    auto allPanels = fetchAllPanels();
    for (int i=0; i<allPanels.size(); i++) {
        if(allPanels[i].id == id) {
            allPanels.removeAt(i);
            break;
        }
    }

    QByteArray saveData = Panel::ExportJSON(allPanels);

    QFile saveFile(PANELSFILE);

    if(saveFile.open(QFile::WriteOnly)) {
        saveFile.write(saveData);
        saveFile.close();
    }

}


Panel Panel::fromDB(int theid)
{

    auto allPanels = fetchAllPanels();
    for (int i=0; i<allPanels.size(); i++) {
        if(allPanels[i].id == theid) {
            return allPanels[i];
        }
    }

    return Panel();
}

Panel Panel::getLaunchPanel()
{

    auto allPanels = fetchAllPanels();
    for (int i=0; i<allPanels.size(); i++) {
        if(allPanels[i].isLaunchPanel()) {
            QDEBUGVAR(allPanels[i].name);
            return allPanels[i];
        }
    }

    return Panel();
}

int Panel::newPanelID(QList<Panel> allPanels)
{
    QList<int> allIDs;
    if(allPanels.empty()) {
        allPanels = fetchAllPanels();
    }
    for (int i=0; i<allPanels.size(); i++) {
        allIDs.append(allPanels[i].id);
    }

    for (int newid=1; newid<65000; newid++) {
        if(!allIDs.contains(newid)) {
            return newid;
        }
    }

    return qrand();

}


Panel Panel::fromDB(QString thename)
{

    auto allPanels = fetchAllPanels();
    for (int i=0; i<allPanels.size(); i++) {
        if(allPanels[i].name == thename) {
            return allPanels[i];
        }
    }

    return Panel();
}


void Panel::init(QList<Packet> packets)
{
    QDEBUGVAR(packets.size());
    buttonList.clear();
    foreach(Packet pkt, packets) {
        PanelButton pb;
        pb.title = pkt.name;
        pb.script = pkt.name;
        buttonList.append(pb);
    }
}
