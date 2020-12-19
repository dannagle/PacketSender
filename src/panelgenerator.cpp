#include "panelgenerator.h"
#include "ui_panelgenerator.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QProcess>
#include <QFontDatabase>
#include <QResource>

#include "tcpthread.h"
#include "globals.h"
#include <QDebug>

#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <random>

extern void themeTheButton(QPushButton * button);

PanelGenerator::PanelGenerator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PanelGenerator)
{
    ui->setupUi(this);

    setWindowTitle("Packet Sender Panels");
    QIcon mIcon("://pslogo.png");
    setWindowIcon(mIcon);

    clearLayout();
    panel.buttonList.clear();

    loadPanelMenu = new QMenu("Load Panel");
    deletePanelMenu = new QMenu("Delete Panel");
    ui->menuLoad->addMenu(loadPanelMenu);
    ui->menuSettings->addMenu(deletePanelMenu);


    editToggleButton = new QPushButton("Viewing");
    themeTheButton(editToggleButton);
    editToggleButton->setIcon(QIcon(PSLOGO));


    packetNetwork = nullptr;

#ifdef RENDER_ONLY
    QDEBUG() << " panel RENDER_ONLY mode";
#else
    QDEBUG() << " panel building mode";

#endif


    QString loadPath = "";

#ifdef RENDER_ONLY
    QDEBUG() << " start packet network for render-only mode";
    packetNetwork = new PacketNetwork(this);
    packetNetwork->init();

    QDEBUG() << " packet send connect attempt:" << connect(this, SIGNAL(sendPacket(Packet)),
             packetNetwork, SLOT(packetToSend(Packet)));

    statusBar()->showMessage("Version " SW_VERSION, 3000);

#else
    QString resLocation = QCoreApplication::applicationDirPath() + "/";

#ifdef __APPLE__
    loadPath = QCoreApplication::applicationDirPath() + "/../Resources/distropackages.rcc";
    bool resourceOK = QResource::registerResource(loadPath);
    QDEBUGVAR(resourceOK);


#else

    loadPath = QCoreApplication::applicationDirPath() + "/distropackages.rcc";
    bool resourceOK = QResource::registerResource(loadPath);
    QDEBUGVAR(resourceOK);
    if(!resourceOK) {
        loadPath = QCoreApplication::applicationDirPath() + "/../../src/distropackages.rcc";
        resourceOK = QResource::registerResource(loadPath);
        QDEBUGVAR(resourceOK);
    }

#endif




#endif


#ifdef RENDER_ONLY
    //remove menu options.
    ui->menuLoad->deleteLater();
    ui->menuSettings->deleteLater();
    ui->menuExport->deleteLater();

#endif



    addLinkButton = new QPushButton("+");
    themeTheButton(addLinkButton);

    connect(addLinkButton, &QPushButton::clicked, this, [this]{
        if(isViewing) {
            //what should i do?

        } else {

            bool ok = false;
            QString linkURL = QInputDialog::getText(this, tr("URL location"),
                                                 tr("URL location:"), QLineEdit::Normal,
                                                 "", &ok);
            if (ok && (!linkURL.isEmpty())) {

                QString linkText = QInputDialog::getText(this, tr("Link text"),
                                                     tr("Link text:"), QLineEdit::Normal,
                                                         "", &ok);

                if (ok && (!linkText.isEmpty())) {
                    QPair<QString, QString> p;
                    p.first = linkText;
                    p.second = linkURL;
                    this->parseEditView();
                    addLink(p);
                    this->renderEditMode();

                    statusBar()->showMessage("Link added. Don't forget to save!", 4000);
                }
            }

        }
    });



    if (!connect(editToggleButton, &QPushButton::clicked, this, &PanelGenerator::editToggle)) {
        QDEBUG() << "editToggleButton connection false";
    }

    statusBar()->insertPermanentWidget(0, editToggleButton);
    statusBar()->insertPermanentWidget(1, addLinkButton);

    isViewing = false;

    renderEditMode();



}



void PanelGenerator::clearLayout()
{

    auto centralLayout = ui->gridLayout;

    if(centralLayout != nullptr) {
        for (int i = 0; i < centralLayout->count(); ++i) {
            QWidget *w = centralLayout->itemAt(i)->widget();
            if(w != nullptr) {
                w->deleteLater();
            }
        }
    }

    foreach(QWidget * w, statusBarWidgets) {
        statusBar()->removeWidget(w);
    }
    statusBarWidgets.clear();

    //statusBar()->permin


}


void PanelGenerator::renderViewMode()
{

    clearLayout();
    isViewing = true;
    setHeaders();
    addLinkButton->hide();

    int i = 0;
    foreach(PanelButton pb, panel.buttonList) {
        QPushButton * const btn = new QPushButton(pb.title);
        themePanelButton(btn);
        QDEBUGVAR(btn->text());
        btn->setProperty("script", pb.script);

        if (!connect(btn, &QPushButton::clicked, this, &PanelGenerator::button_clicked)) {
            QDEBUG() << "clicked connection false";
        }

        i++;
        ui->gridLayout->addWidget(btn, (i-1) / 3, (i-1) % 3);
    }


    for(int j = 0; j < panel.linkTexts.size(); j++) {
        QString link = panel.linkTexts[j];
        QString url = panel.linkURLs[j];
        QPushButton * const btn = new QPushButton(link);
        btn->setProperty("w-url", url);
        themeTheButton(btn);
        connect(btn, &QPushButton::clicked, this, [btn]{
            QString url = btn->property("w-url").toString();
            if(!url.startsWith("http")) {
                url.prepend("http://");
            }
            //QDEBUGVAR(url);
            QDesktopServices::openUrl(QUrl(url));

        });

        statusBar()->insertPermanentWidget(j+2, btn);
        statusBarWidgets.append(btn);
    }



}

void PanelGenerator::setHeaders()
{
#ifdef RENDER_ONLY
    isViewing = true;
    editToggleButton->hide();
#endif

    if(!isViewing) {

        auto actions = loadPanelMenu->actions();
        foreach(QAction *m, actions) {
            loadPanelMenu->removeAction(m);
        }
        actions = deletePanelMenu->actions();
        foreach(QAction *m, actions) {
            deletePanelMenu->removeAction(m);
        }


        auto menuList = Panel::fetchAllPanels();
        QDEBUGVAR(menuList.size());
        QString checkMark = "(a)"; //QChar(0x2713);
        foreach(Panel p, menuList) {
            QString s = checkMark + " ";
            if(!p.isLaunchPanel()) {
                s.clear();
            }

            if(panel.id == p.id) {
                s.prepend(" ");
                s.prepend(QChar(0x2713));
            }

            auto action = loadPanelMenu->addAction(s + QString::number(p.id) + ": " + p.name + " ("+p.lastmodfied+") ");
            action->setProperty("w-id", p.id);
            connect(action, &QAction::triggered, this, [action, this]{
                int panelID = action->property("w-id").toInt();
                if(panelID > 0) {
                    Panel p = Panel::fromDB(panelID);
                    if(p.id == panelID) {
                        panel.copy(p);
                        renderEditMode();
                        statusBar()->showMessage("Loading " + panel.name, 2000);
                        QDEBUGVAR(panel.toString());
                    }
                }
            });
            action = deletePanelMenu->addAction(action->text());
            action->setProperty("w-id", p.id);
            connect(action, &QAction::triggered, this, [action, this]{
                int panelID = action->property("w-id").toInt();
                if(panelID > 0) {
                    Panel p = Panel::fromDB(panelID);
                    if(p.id == panelID) {
                        QMessageBox msgBox;
                        msgBox.setWindowIcon(QIcon(":pslogo.png"));
                        msgBox.setWindowTitle("Confirm delete.");
                        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        msgBox.setDefaultButton(QMessageBox::Yes);
                        msgBox.setIcon(QMessageBox::Information);
                        msgBox.setText("Delete " + p.name + ", ID:" + QString::number(p.id));
                        int yesno = msgBox.exec();
                        if (yesno == QMessageBox::Yes) {
                            QDEBUG() << "Delete";
                            p.deleteFromDB();
                            setHeaders();
                            statusBar()->showMessage("Deleting " + p.name, 2000);
                        }
                    }

                }
            });
        }

        setWindowTitle("Panel " + QString::number(panel.id) + ": " + panel.name + " ("+panel.lastmodfied+")");

        ui->actionPanel_Name->setText("Set Panel Name: "+panel.name);
        ui->actionPanel_ID->setText("Set Panel ID: "+QString::number(panel.id));
        ui->actionDelete_Panel->setEnabled(true);

        if(panel.id == 0) {
            setWindowTitle("Packet Sender Panel: New Panel");
            ui->actionPanel_Name->setText("Set Panel Name: New Panel");
            ui->actionPanel_ID->setText("Set Panel ID: New Panel");
            ui->actionDelete_Panel->setEnabled(false);
        }


        if(panel.isLaunchPanel()) {
            ui->actionLaunch_Panel->setText("Auto-launch: Yes");
        } else {
            ui->actionLaunch_Panel->setText("Auto-launch: No");
        }
        ui->menubar->show();
        editToggleButton->setText("Editing");

    } else {
        setWindowTitle(panel.name);
        ui->menubar->hide();
        editToggleButton->setText("Viewing");
    }

}

void PanelGenerator::themePanelButton(QPushButton *button)
{

    // TODO: make this font resize based on window size.

    int id = QFontDatabase::addApplicationFont("://OpenSans-Regular.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont opensans(family);

    auto label = new QLabel(button->text(),button);
    button->setText("");
    label->setWordWrap(true);
    auto layout = new QHBoxLayout(button);

    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setFont(opensans);

    label->setAutoFillBackground(true);
    label->setStyleSheet("QLabel { font-size: 20pt; color: white; background-color: transparent;} QLabel::hover { color: #BC810C; } ");
    label->setCursor(Qt::PointingHandCursor);
    button->update();
    label->update();
    layout->addWidget(label,0,Qt::AlignCenter);

}


void PanelGenerator::renderEditMode(bool last /* = false */)
{

    clearLayout();
    isViewing = false;
    setHeaders();
    panel.sortButtons();

    addLinkButton->show();


/*
Target layout.

[menu bar]


<title> [x]
____________
Script
____________
[testbutton]


*/

    QTextEdit * textEditFirst = nullptr;
    QTextEdit * textEditLast = nullptr;

    PanelButton pb;
    int i = 0;
    foreach(pb, panel.buttonList) {

        auto groupBox = new QGroupBox;
        groupBox->setTitle("Button "+ QString::number(pb.id));


        auto hbox = new QHBoxLayout;
        auto vbox = new QVBoxLayout;

        auto title = new QLineEdit(pb.title);
        title->setPlaceholderText("(Button will be deleted)");
        QChar thickX = QChar(0x2716);
        auto xButton = new QPushButton(thickX);

        if (!connect(xButton, &QPushButton::clicked, this, [title]() {
                     title->clear();
                 })) {
            QDEBUG() << "testButton connection false";
        }

        auto testButton = new QPushButton("Test");

        xButton->setFixedSize(24, 24);
        hbox->addWidget(title);
        hbox->addWidget(xButton);

        QTextEdit *  textEdit = new QTextEdit;
        textEdit->setPlaceholderText("Script is empty");
        if(!pb.script.isEmpty()) {
            textEdit->setText(pb.script.trimmed()+"\r\n");
        }


        setMetaData(pb, textEdit);
        setMetaData(pb, testButton);
        setMetaData(pb, title);
        setMetaData(pb, xButton);
        setMetaData(pb, groupBox);

        QDEBUGVAR(textEdit->toPlainText());
        textEdit->setFixedSize(200, 50);
        textEditLast = textEdit;
        if(i == 0) {
            textEditFirst = textEdit;
        }

        i++;

        if (!connect(testButton, &QPushButton::clicked, this, &PanelGenerator::testButtonClicked)) {
            QDEBUG() << "testButton connection false";
        }


        vbox->addLayout(hbox);
        vbox->addWidget(textEdit);
        vbox->addWidget(testButton);
        vbox->setMargin(10);

        groupBox->setLayout(vbox);



        ui->gridLayout->addWidget(groupBox, (i-1) / 3, (i-1) % 3);


    }

    //load up the button edits
    for(int j = 0; j < panel.linkTexts.size(); j++) {
        QString link = panel.linkTexts[j];
        QString url = panel.linkURLs[j];
        QPushButton * const btn = new QPushButton("* " + link);
        btn->setProperty("w-url", url);
        btn->setProperty("w-link", link);
        btn->setProperty("w-index", j);
        themeTheButton(btn);

//statusbar_clicked
        if (!connect(btn, &QPushButton::clicked, this, &PanelGenerator::statusbar_clicked)) {
            QDEBUG() << "btn connection false";
        }

        statusBar()->insertPermanentWidget(j+2, btn);
        statusBarWidgets.append(btn);
    }


    i++;
    //add new item button.
    auto newButton = new QPushButton("Add New Button");

    connect(newButton, &QPushButton::clicked, this, [this]{
       PanelButton pb;
       pb.id = 1;
       pb.title = "New Button";
       if(!panel.buttonList.isEmpty()) {
           pb.id = panel.buttonList.last().id + 1;
       }
       panel.buttonList.append(pb);
       renderEditMode(true);
    });
    ui->gridLayout->addWidget(newButton, (i-1) / 3, (i-1) % 3);

    if(last) {
        textEditFirst = textEditLast;
    }

    if(textEditFirst != nullptr) {
        textEditFirst->setFocus();
        QTextCursor tmpCursor = textEditFirst->textCursor();
        tmpCursor.movePosition(QTextCursor::End);
        textEditFirst->setTextCursor(tmpCursor);

    }





}

PanelButton PanelGenerator::fromMetaData(QWidget * w)
{
    PanelButton pb;
    pb.title = w->property("w-title").toString();
    pb.script = w->property("w-script").toString();
    pb.id = w->property("w-id").toInt();

    return pb;

}
void PanelGenerator::setMetaData(PanelButton p, QWidget * w)
{
    w->setProperty("w-id", p.id);
    w->setProperty("w-script", p.script);
    w->setProperty("w-title", p.title);
}


void PanelGenerator::init(QList<Packet> packets)
{
    QDEBUGVAR(packets.size());
    panel.buttonList.clear();
    panel.id = 1;
    panel.name = "Test Panel";
    int buttonID = 1;
    foreach(Packet pkt, packets) {
        PanelButton pb;
        pb.title = pkt.name;
        pb.script = pkt.name;
        pb.id = buttonID;
        panel.buttonList.append(pb);
        buttonID++;
    }
    renderViewMode();

}

void PanelGenerator::testButtonClicked()
{
    QDEBUG();

    PanelButton pb1, pb2;

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if( button != nullptr ) {
        pb1 = fromMetaData(button);
        if(pb1.id > 0){
            QDEBUG() << "Need to send list from ID" << pb1.id ;

            auto list = ui->centralwidget->findChildren<QTextEdit *>();
            if(!list.isEmpty()) {
                QTextEdit * textEdit;
                foreach(textEdit, list) {
                    pb2 = fromMetaData(textEdit);
                    if(pb1.id == pb2.id) {
                        QString script = textEdit->toPlainText();
                        executeScript(script);
                        break;
                    }
                }
            } else {
                QDEBUGVAR(list.size());
            }
        }
    }
}

void PanelGenerator::executeScript(QString script)
{
    QStringList linesAll = script.split("\n", QString::SkipEmptyParts);

    QString errorMessages = "";

    QStringList lines;
    int toLoad = 0;

    foreach(QString line, linesAll) {
        line = line.trimmed();
        if( line.isEmpty() ||
                line.startsWith("//") ||
                line.startsWith("#") ||
                line.startsWith(";")
                ) {
            continue;
        }

        QStringList cmdSplit = line.split(":");

        bool ispacket = true;

        //TODO: support system calls.
        if( (line.startsWith("sleep:")  || line.startsWith("delay:") || line.startsWith("panel:") ) ) {

            ispacket = false;
            if(cmdSplit.size() < 2) {
                errorMessages.append("\nInvalid command:" + line);
                continue;
            }

            unsigned int delaynum = cmdSplit[1].toUInt();
            if(delaynum < 1) {
                errorMessages.append("\nInvalid command:" + line);
                continue;
            }
        }

        if( line.startsWith("panel:") && cmdSplit.size() > 1) {
            int panelnum = cmdSplit[1].toInt();
            Panel p = Panel::fromDB(panelnum);
            if(p.id == 0) {
                errorMessages.append("\nInvalid panel id:" + line);
                continue;
            } else {
                toLoad = p.id;

            }
        }

        if(ispacket) {
            Packet p = Packet::fetchFromDB(line);
            if(p.name.isEmpty()) {
                errorMessages.append("\nUnknown packet:" + line);
            }
        }

        lines.append(line);
    }

    if(lines.isEmpty()) {
        statusBar()->showMessage("Nothing to do...", 2000);
    } else {
        statusBar()->showMessage("Executing ", 2000);
    }

    if(!errorMessages.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Unknown Command");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(errorMessages.trimmed());
        msgBox.exec();
        return;
    }

    QtConcurrent::run([this, lines, toLoad]{
        QList<Packet> packetList = Packet::fetchAllfromDB("");


        QDEBUG();

        //already prepped

        foreach(QString line, lines) {
            QDEBUGVAR(line);

            QStringList cmdSplit = line.split(":");

            if( (line.startsWith("sleep:")  || line.startsWith("delay:"))) {
                unsigned int delaynum = cmdSplit[1].toUInt();
                QDEBUG() << "Delaying" << delaynum;
                QThread::sleep(delaynum);
                continue;
            }
            QDEBUG();

            if( line.startsWith("panel:")) {
                int panelnum = cmdSplit[1].toInt();
                Panel p = Panel::fromDB(panelnum);
                if(p.id > 0) {
                    QDEBUG() << "load panel" << p.name;
                } else {
                    QDEBUG() << "unknown panel" << p.name;
                }
                continue;
            }


            QDEBUG();

            Packet pkt = Packet::fetchFromList(line, packetList);
            if(pkt.name == line) {
                QDEBUG() << "Need to send packet" << pkt.name;
                emit sendPacket(pkt);
                QThread::msleep(100); //give time to send
            }

            QDEBUG();
        }
        //QThread::sleep(10);

        QDEBUG();
        QObject src;
        src.connect(&src, &QObject::destroyed, this, [this, toLoad]{
           QDEBUG() << "Finished script";
           statusBar()->showMessage("Finished script", 2000);
           if(toLoad > 0) {
               QDEBUG() << "Need to load new panel" << toLoad;
               Panel p = Panel::fromDB(toLoad);
               if(p.id > 0) {
                   if(isViewing) {
                       bool loadPanel = true;
#ifndef RENDER_ONLY
                       loadPanel = false;
                       QMessageBox msgBox;
                       msgBox.setWindowTitle("Transition Panel");
                       msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                       msgBox.setDefaultButton(QMessageBox::No);
                       msgBox.setIcon(QMessageBox::Warning);
                       msgBox.setText("Tranisition to Panel "+p.name + "?\nYou may lose unsaved changes.");
                       int yesno = msgBox.exec();
                       if (yesno == QMessageBox::Yes) {
                            loadPanel = true;
                       }

#endif

                       if(loadPanel) {
                           this->panel.copy(p);
                           this->renderViewMode();
                       } else {
                           statusBar()->showMessage("Not loading panel", 4000);
                       }

                   } else {
                       statusBar()->showMessage("Not loading panel in edit mode", 4000);
                   }
               }
           }

        });
        // see https://stackoverflow.com/q/21646467/1329652 for better ways
        // of invoking doNext
    });

}

void PanelGenerator::initAutoLaunch()
{
    panel.copy(Panel::getLaunchPanel());

    /*
    QDEBUGVAR(this->panel.linkTexts);
    QPair<QString, QString> p;
    p.first = "text";
    p.second = "url";
    QDEBUG() << "Saving";
    addLink(p);
    on_actionSave_triggered();
    QDEBUG() << "Done Saving";
    QDEBUGVAR(this->panel.linkTexts);

    */

    renderViewMode();
}

void PanelGenerator::initAutoLaunchOrEditMode()
{
    panel.copy(Panel::getLaunchPanel());
    QDEBUGVAR(panel.toString());
    if(panel.id == 0) {
        renderEditMode();
    } else {
        renderViewMode();
    }

}


void PanelGenerator::parseEditView()
{

    auto textList = ui->centralwidget->findChildren<QTextEdit *>();
    auto lineList = ui->centralwidget->findChildren<QLineEdit *>();
    QHash<int, PanelButton> buttonMap;

    QTextEdit * textEdit;
    PanelButton pb;
    foreach(textEdit, textList) {
        pb = fromMetaData(textEdit);
        if(pb.id > 0) {
            buttonMap[pb.id].id = pb.id;
            buttonMap[pb.id].script = textEdit->toPlainText().trimmed();
        }
    }

    QLineEdit * lineEdit;
    foreach(lineEdit, lineList) {
        pb = fromMetaData(lineEdit);
        if(pb.id > 0) {
            buttonMap[pb.id].id = pb.id;
            buttonMap[pb.id].title = lineEdit->text().trimmed();
        }
    }

    QList<int> keys = buttonMap.keys();
    int key;
    panel.buttonList.clear();
    foreach(key, keys) {
        if(!buttonMap[key].title.isEmpty()) {
            panel.buttonList.append(buttonMap[key]);
        }
    }


}

void PanelGenerator::editToggle()
{
    QDEBUG();

    if(isViewing) {
        renderEditMode();

    } else {

        parseEditView();
        renderViewMode();

    }

}



void PanelGenerator::statusbar_clicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if( btn != nullptr ) {
        QString url = btn->property("w-url").toString();
        QString urltext = btn->property("w-link").toString();
        int index = btn->property("w-index").toInt();
        QDEBUGVAR(url);

        bool ok = false;
        QPair<QString, QString> link;
        link.first = urltext;
        link.second = url;
        QString text = QInputDialog::getText(this, tr("Change URL"),
                                             tr("URL:"), QLineEdit::Normal,
                                             link.second, &ok);
        if (ok && !text.isEmpty()) {
            link.second = text.trimmed();
        } else {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Delete link");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Delete URL link?\n"+urltext + "->" + url);
            int yesno = msgBox.exec();

            if (yesno == QMessageBox::Yes) {
                parseEditView();
                deleteLink(index);
                renderEditMode();
            }
            return;
        }

        text = QInputDialog::getText(this, tr("Change Text"),
                                             tr("text:"), QLineEdit::Normal,
                                             link.first, &ok);
        if (ok && !text.isEmpty()) {
            link.first = text;
        }

        parseEditView();
        editLink(link, index);
        renderEditMode();


    }
}


void PanelGenerator::button_clicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if( button != nullptr ) {
        QString script = button->property("script").toString();        
        executeScript(script);
    }
}

PanelGenerator::~PanelGenerator()
{
    delete ui;
}

void PanelGenerator::on_actionDocumentation_triggered()
{
    //Open URL in browser
    QDesktopServices::openUrl(QUrl("https://packetsender.com/documentation"));

}


void PanelGenerator::on_actionSave_triggered()
{
    QDEBUG();
    if(!isViewing) {
        parseEditView();
    }
    if(panel.id != 0) {
        panel.saveToDB();
        setHeaders();
    } else {
        on_actionSave_As_triggered();
    }
}

void PanelGenerator::on_actionImport_File_triggered()
{
    QDEBUG();

    static QString fileName;

    if (fileName.isEmpty()) {
        fileName = QDir::homePath();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Panels File"),
                                            fileName,
                                            tr("*.panels"));

    if (fileName.isEmpty()) {
        return;
    }

    QDEBUGVAR(fileName);


    QString packetsjson, panelsjson;
    packetsjson.clear();
    panelsjson.clear();

    QFile loadFile(fileName);
    if(loadFile.open(QFile::ReadOnly)) {
        QString contents = QString(loadFile.readAll());
        loadFile.close();
        QStringList split = contents.split(PACKETS_PANELS_DELIM, QString::SkipEmptyParts);
        if(split.size() == 2) {
            packetsjson = split[0];
            panelsjson = split[1];
        }

    }

    if(packetsjson.isEmpty() || panelsjson.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setWindowTitle("Unknown File");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("This does not seem to be a valid PS Panels file");
        msgBox.exec();
        return;

    }

    auto pList = Packet::ImportJSON(packetsjson.toUtf8());
    if(!pList.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setWindowTitle(loadFile.fileName());
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Found " + QString::number(pList.size()) + " packets. Saving overwrites duplicate names.\n\nContinue packet import?");
        int yesno = msgBox.exec();

        if (yesno == QMessageBox::Cancel) {
            return;
        }

        if (yesno == QMessageBox::Yes) {
            foreach(Packet pkt, pList) {
                pkt.saveToDB();
            }
        }
    }


    auto savedList = Panel::fetchAllPanels();
    auto importList = Panel::ImportJSON(panelsjson.toUtf8());

    if(!importList.isEmpty()) {

        QMessageBox msgBox;
        msgBox.setWindowTitle(loadFile.fileName());
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Found " + QString::number(importList.size()) + " panels. Ovewrite dpulicated IDs?\n\nSelecting No will generate new IDs and append.");
        int yesno = msgBox.exec();

        foreach(Panel p, savedList) {
            QDEBUG() << "Before:" << p.id << p.name;
        }
        if (yesno == QMessageBox::Cancel) {
            return;
        }
        QHash<int, Panel> panelMap;
        foreach(Panel p, savedList) {
            panelMap[p.id].copy(p);
        }

        if (yesno == QMessageBox::Yes) {
            //overwrite
            foreach(Panel p, importList) {
                panelMap[p.id].copy(p);
            }

        }
        if (yesno == QMessageBox::No) {
            //append
            foreach(Panel p, importList) {
                QDEBUGVAR(p.id);
                if(panelMap[p.id].id > 0) {
                    p.id = Panel::newPanelID(savedList);
                    savedList.append(p);
                }
                QDEBUGVAR(p.id);
                panelMap[p.id].copy(p);
            }
        }

        auto intList = panelMap.keys();
        savedList.clear();
        foreach(int i, intList) {
            savedList.append(panelMap[i]);
        }

    }

    foreach(Panel p, savedList) {
        QDEBUG() << "After:" << p.id << p.name;
    }

    QFile saveFile(PANELSFILE);

    if(saveFile.open(QFile::WriteOnly)) {
        saveFile.write(Panel::ExportJSON(savedList));
        saveFile.close();
        renderEditMode();
    }


}

void PanelGenerator::showFileInFolder(const QString &path){


    #ifdef Q_OS_WIN    //Code for Windows
        QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(path)});
    #elif defined(__APPLE__)    //Code for Mac
        QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + path + "\""});
        QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
    #endif
}


void PanelGenerator::buildPackage(bool isMac)
{

    QMessageBox msgBox(this);
    msgBox.setText("Coming soon.");
    msgBox.exec();

}




void PanelGenerator::on_actionWin_Package_triggered()
{
    buildPackage(false);

}

void PanelGenerator::on_actionPanel_Name_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Change Name"),
                                         tr("Name:"), QLineEdit::Normal,
                                         panel.name, &ok);
    if (ok && !text.isEmpty()) {        
        panel.name = text;
        panel.saveToDB();
        setHeaders();
    }


}

void PanelGenerator::on_actionPanel_ID_triggered()
{
    QDEBUG();
    QString oldID = ui->actionPanel_ID->property("w-id").toString();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Change ID"),
                                         tr("ID:"), QLineEdit::Normal,
                                         oldID, &ok);
    bool okID;
    if (ok && !text.isEmpty()) {
        int newid = text.toInt(&okID);
        if(okID) {
            parseEditView();
            int oldId = panel.id;
            panel.id = newid;
            panel.saveToDB();
            Panel tmpP;
            tmpP.id = oldId;
            tmpP.deleteFromDB();
            renderEditMode();
        }
    }


}

void PanelGenerator::on_actionSave_As_triggered()
{
    QDEBUG();
    bool ok;
    QString oldName = panel.name  + " - Copy";
    if(panel.id == 0) {
        oldName = "Panel " + QDateTime::currentDateTime().toString("yyyy-MM-dd");
    }


    QString text = QInputDialog::getText(this, tr("New Name"),
                                         tr("Name:"), QLineEdit::Normal,
                                         oldName, &ok);

    if (ok && !text.isEmpty()) {
        parseEditView();
        panel.id = Panel::newPanelID(QList<Panel>());
        panel.name = text;
        panel.saveToDB();
        renderEditMode();
    }
}

void PanelGenerator::on_actionLaunch_Panel_triggered()
{
    if(panel.isLaunchPanel()) {
        panel.launch = 0;
        statusBar()->showMessage("Panel is no longer launch panel", 2000);
    } else {
        panel.launch = 1;
        statusBar()->showMessage("panel is now launch panel", 2000);
    }

    panel.saveToDB();
    setHeaders();


}

void PanelGenerator::on_actionPanels_File_triggered()
{
    QDEBUG() <<"Need to launch save file dialog";

    static QString fileName = QDir::homePath() +"/" + panel.name + ".panels";

    fileName = QFileDialog::getSaveFileName(this, tr("Save PS Panels"),
                                            QDir::toNativeSeparators(fileName), tr("panels (*.panels)"));


    if (fileName.isEmpty()) {
        return;
    }

    //TODO:only save actually needed packet file.

    QFile saveFile(fileName);
    if (saveFile.open(QFile::WriteOnly)) {
        saveFile.write(Packet::ExportJSON(Packet::fetchAllfromDB("")));
        saveFile.write("\n");
        saveFile.write(PACKETS_PANELS_DELIM);
        saveFile.write("\n");
        saveFile.write(Panel::ExportJSON(Panel::fetchAllPanels()));
        saveFile.close();
    }


    statusBar()->showMessage("Saved " + fileName, 2000);

}


void PanelGenerator::addLink(QPair<QString, QString> link)
{
    QDEBUGVAR(link);
    this->panel.linkTexts.append(link.first);
    this->panel.linkURLs.append(link.second);
    QDEBUGVAR(panel.linkTexts.size());
}

void PanelGenerator::deleteLink(int index)
{
    QDEBUGVAR(index);
    if(this->panel.linkTexts.size() > index) {
        this->panel.linkTexts.removeAt(index);
        this->panel.linkURLs.removeAt(index);
    } else {
        QDEBUG() << "abandon deleteLink job";
    }

}

void PanelGenerator::editLink(QPair<QString, QString> link, int index)
{
    QDEBUG() << "edit"<<index << (link);

    if(this->panel.linkTexts.size() > index) {
        this->panel.linkTexts.replace(index, link.first);
        this->panel.linkURLs.replace(index, link.second);
    } else {
        QDEBUG() << "abandon editLink job";
    }

}


void PanelGenerator::on_actionMac_Package_triggered()
{
    buildPackage(true);

}

void PanelGenerator::on_actionLinux_Package_triggered()
{
    buildPackage(true);
}
