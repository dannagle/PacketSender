#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QList>
#include <QPair>

#include "packet.h"
#include "panel.h"

#include "packetnetwork.h"


namespace Ui {
class PanelGenerator;
}

class PanelGenerator : public QMainWindow
{
    Q_OBJECT

public:
    explicit PanelGenerator(QWidget *parent = nullptr);
    ~PanelGenerator();

    void init(QList<Packet> packets);
    void executeScript(QString script);
    void initAutoLaunch();
    void initAutoLaunchOrEditMode();

    void addLink(QPair<QString, QString> link);
    void deleteLink(int index);
    void editLink(QPair<QString, QString> link, int index);

    static void showFileInFolder(const QString &path);
signals:
    void sendPacket(Packet sendpacket);

private slots:
    void on_actionDocumentation_triggered();

    void on_actionSave_triggered();

    void on_actionImport_File_triggered();

    void on_actionWin_Package_triggered();

    void on_actionPanel_Name_triggered();

    void on_actionPanel_ID_triggered();

    void on_actionSave_As_triggered();

    void on_actionLaunch_Panel_triggered();


    void on_actionPanels_File_triggered();

    void on_actionMac_Package_triggered();

    void on_actionLinux_Package_triggered();

private:
    Ui::PanelGenerator *ui;
    void button_clicked();
    void statusbar_clicked();
    Panel panel;
    void editToggle();
    bool isViewing;
    void renderViewMode();
    void renderEditMode(bool last = false);
    QPushButton * editToggleButton;
    QPushButton * addLinkButton;
    void clearLayout();
    void testButtonClicked();
    void parseEditView();
    void setMetaData(PanelButton p, QWidget *w);
    PanelButton fromMetaData(QWidget *w);
    QMenu * loadPanelMenu;
    QMenu * deletePanelMenu;
    void setHeaders();
    //Qt does not keep track of statusbar widgets, so i must do so.
    QList<QWidget *> statusBarWidgets;
    void themePanelButton(QPushButton *button);
    void buildPackage(bool isMac);

    PacketNetwork * packetNetwork;


    QPushButton *createQPushButtonWithWordWrap(QWidget *parent, const QString &text);
};

