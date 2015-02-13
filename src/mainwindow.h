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
    void applyNetworkSettings();
signals:
    void sendPacket(Packet sendpacket);

public slots:
    void gotoPacketSenderDotCom();
    void gotoDanNagleDotCom();
    void gotoDanNaglePayPal();
    void gotoNagleCode();
    void toTrafficLog(Packet logPacket);
    void cancelResends();
    void dragDropEvent();
    void poodlepic();


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

    void on_tcpServerPortEdit_textEdited(const QString &arg1);

    void on_udpServerPortEdit_textEdited(const QString &arg1);

    void on_sendResponseSettingsCheck_toggled(bool checked);

    void on_responsePacketBox_activated(int index);

    void on_udpServerEnableCheck_toggled(bool checked);

    void on_tcpServerEnableCheck_toggled(bool checked);

    void toggleUDPServer();
    void toggleTCPServer();


    void on_packetASCIIEdit_editingFinished();

    void on_packetHexEdit_editingFinished();

    void on_packetASCIIEdit_textEdited(const QString &arg1);

    void on_packetIPEdit_editingFinished();

    void on_hexResponseEdit_editingFinished();

    void on_asciiResponseEdit_editingFinished();

    void on_responsePacketBox_currentIndexChanged(const QString &arg1);

    void on_searchLineEdit_textEdited(const QString &arg1);

    void on_clearSearch_clicked();

    void on_displayOrderList_indexesMoved(const QModelIndexList &indexes);

    void on_displayOrderList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_searchTrafficEdit_textEdited(const QString &arg1);

    void on_clearTrafficSearchButton_clicked();

    void on_toClipboardButton_clicked();

    void on_defaultDisplayButton_clicked();

    void on_packetsTable_itemSelectionChanged();

    void on_attemptReceiveCheck_clicked(bool checked);

    void on_rolling500entryCheck_clicked(bool checked);

    void on_copyUnformattedCheck_clicked(bool checked);

    void on_delayAfterConnectCheck_clicked(bool checked);

    void on_persistentConnectCheck_clicked(bool checked);

    void on_bugsLinkButton_clicked();

    void on_forumsPacketSenderButton_clicked();


    void on_exportPacketsButton_clicked();

    void on_importPacketsButton_clicked();

    void on_hideQuickSendCheck_clicked(bool checked);

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

    QStringList packetTableHeaders;
    QStringList packetSavedTableHeaders;

    Packet lastSendPacket;

    QList<SendPacketButton *> sendButtonHolder;

    int maxLogSize;


    void setDefaultTableHeaders();
    void setStoredTableHeaders();
    void loadTableHeaders();

};

#endif // MAINWINDOW_H
