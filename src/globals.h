/*
 * This file is part of Packet Sender
 *
 * Licensed GPL v2
 * http://PacketSender.com/
 *
 * Copyright Dan Nagle
 *
 */
#ifndef GLOBALS_H
#define GLOBALS_H

#define QDEBUG() qDebug() << __FILE__ << "/" <<__LINE__  <<"(" << __FUNCTION__ << "):"
#define QDEBUGVAR(var)  QDEBUG() << # var <<  var;

#define DATETIMEFORMAT "h:mm:ss.zzz ap"

#define TEMPPATH  QDir::toNativeSeparators(QDir::temp().absolutePath()) + QDir::separator() + "PacketSender" + QDir::separator()
#define SETTINGSPATH QStandardPaths::writableLocation( QStandardPaths::GenericDataLocation )+ QDir::separator() +"PacketSender" + QDir::separator()
#define SETTINGSFILE SETTINGSPATH  + "ps_settings.ini"
#define PACKETSFILE SETTINGSPATH + "packets.ini"
#define TRAFFICFILE SETTINGSPATH + "traffic.ini"
#define LOGFILE SETTINGSPATH + "packetsender.log"

#define NAMEINIKEY "NAMES"

#define UDPSENDICON ":icons/tx_udp.png"
#define TCPSENDICON ":icons/tx_tcp.png"
#define UDPRXICON ":icons/rx_udp.png"
#define TCPRXICON ":icons/rx_tcp.png"
#define PSLOGO ":pslogo.png"
#define UPDOWNICON ":icons/moveupdown.png"


#endif // GLOBALS_H
