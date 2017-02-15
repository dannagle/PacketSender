/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModelIndex>
#include <QTimer>
#include <QItemSelection>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QListWidget>
#include "globals.h"
#include "packet.h"
#include "packetnetwork.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    quint64 timeSinceLaunch();
    QString ASCIITohex(QString &ascii);
    QString hexToASCII(QString &hex);

    void loadPacketsTable();

    QString hyperLinkStyle;

    QPushButton *generatePSLink();
    QPushButton *generateDNLink();
    void loadTrafficLogTable();
    void populateTableRow(int rowCounter, Packet tempPacket);
    void removePacketfromMemory(Packet thepacket);
    void UDPServerStatus();
    void TCPServerStatus();
    int findColumnIndex(QListWidget *lw, QString search);
    void packetTable_checkMultiSelected();
    void generateConnectionMenu();


signals:
    void sendPacket(Packet sendpacket);

public slots:
    void toTrafficLog(Packet logPacket);
    void cancelResends();
    void applyNetworkSettings();


    void toggleUDPServer();
    void toggleTCPServer();
    void toggleIPv4_IPv6();
    void ebcdicTranslate();

    //shortcut keys... would be better if used lambda
    void poodlepic();
    void shortcutkey1();
    void shortcutkey2();
    void shortcutkey3();
    void shortcutkey4();
    void shortcutkey5();
    void shortcutkey6();
    void shortcutkey7();



private slots:
    void on_packetHexEdit_lostFocus();
    void on_packetASCIIEdit_lostFocus();

    void sendClick(QString packetName);

    void statusBarMessage(const QString & msg, int timeout, bool override);

    void on_savePacketButton_clicked();

    void on_testPacketButton_clicked();

    void on_deletePacketButton_clicked();

    void on_packetIPEdit_lostFocus();

    void on_packetPortEdit_lostFocus();

    void httpFinished(QNetworkReply* pReply);

    void on_packetsTable_itemChanged(QTableWidgetItem *item);

    void on_packetsTable_itemClicked(QTableWidgetItem *item);
    void refreshTimerTimeout();

    void on_trafficLogClearButton_clicked();

    void on_saveTrafficPacket_clicked();


    void on_packetASCIIEdit_editingFinished();

    void on_packetHexEdit_editingFinished();

    void on_packetASCIIEdit_textEdited(const QString &arg1);

    void on_packetIPEdit_editingFinished();

    void on_searchLineEdit_textEdited(const QString &arg1);


    void on_toClipboardButton_clicked();

    void on_packetsTable_itemSelectionChanged();

    void on_bugsLinkButton_clicked();

    void on_forumsPacketSenderButton_clicked();

    void on_saveLogButton_clicked();

    void on_actionAbout_triggered();

    void on_actionHelp_triggered();

    void on_actionSettings_triggered();

    void on_actionExit_triggered();

    void on_actionImport_Packets_triggered();

    void on_actionExport_Packets_triggered();

    void on_persistentTCPCheck_clicked(bool checked);


    void on_actionSubnet_Calculator_triggered();


    void on_resendEdit_editingFinished();

    void on_loadFileButton_clicked();

    void on_actionDonate_Thank_You_triggered();

private:
    Ui::MainWindow *ui;
    QList<Packet> packetsLogged;
    QList<Packet> packetsSaved;
    QList<Packet> packetsRepeat;
    int stopResending;
    PacketNetwork packetNetwork;
    QNetworkAccessManager  * http;
    QTimer refreshTimer;
    bool tableActive;
    QPushButton * udpServerStatus;
    QPushButton * tcpServerStatus;
    QPushButton * stopResendingButton;
    QPushButton * IPmodeButton;

    QString IPv4Stylesheet;
    QString IPv6Stylesheet;


    QStringList packetTableHeaders;
    QStringList packetSavedTableHeaders;

    Packet lastSendPacket;

    int maxLogSize;

    float multiSendDelay;
    int cancelResendNum;
    int resendCounter;

    bool asciiEditTranslateEBCDIC;

    void setIPMode();
    void saveSession(Packet sessionPacket);
};

#endif // MAINWINDOW_H
