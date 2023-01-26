# Einführung

![Paket Sender Logo](screenshots/packetsender_banner.png)


*Lesen Sie dies in anderen Sprachen: [English](README.md), [Español](README.es.md), [Deutsch](README.de.md), [Français](README.fr.md).*


[![herunterladen](https://img.shields.io/github/downloads/dannagle/PacketSender/total.svg)](https://packetsender.com/download)

Paket Sender ist ein Open-Source-Dienstprogramm, das das Senden und Empfangen von TCP-, UDP- und SSL-Paketen (verschlüsseltes TCP) sowie HTTP/HTTPS-Anfragen und die Erstellung von Panels ermöglicht. Der Mainline-Zweig unterstützt offiziell Windows, Mac und Desktop-Linux (mit Qt). Andere Stellen können Paket Sender neu kompilieren und weiterverteilen. Paket Sender ist kostenlos und steht unter der GPL v2 oder höher. Es kann sowohl für kommerzielle als auch für private Zwecke verwendet werden. Wenn Sie die App nützlich finden, ziehen Sie bitte eine Spende/Sponsoring in Betracht, damit die Entwicklung fortgesetzt werden kann.



# Inhaltsverzeichnis
* [Sponsoren](#sponsors)
  * Besuchen Sie [IWL.com](https://www.iwl.com/)
  *  Besuchen Sie  [NagleCode.com](https://dannagle.com/)

* [GUI](#gui)
* [Intensiver Verkehrsgenerator (GUI)](#udptraffic)
* [Netzwerk](#network)
* [Herunterladen](#downloads)
* [Unterstützung](#support)
* [IPv4-Subnetz-Rechner](#subnetcalculator)
* [Paket Sender Cloud](#cloud)
* [Tragbarer Modus](#portable)
* [Makros und intelligente Antworten](#smartresponses)
* [Persistentes TCP und SSL](#persistent)
* [HTTP/HTTPS-Anfragen](#http)
* [Panel Generator](#panelgen)
* [Befehlszeilenschnittstelle](#cli)
* [Intensiver Verkehrsgenerator (CLI)](#cliintensetraffic)
* [Gebäude-Paket-Sender](#building)




<a id="Sponsoren"></a>
## Sponsors

Paket Sender möchte sich bei den folgenden Sponsoren bedanken.

[![IWL](screenshots/iwl_logo.png)](https://www.iwl.com/)
<br>IWL ist ein kalifornisches Unternehmen, das Computer-Netzwerkprodukte herstellt.
<br><br><br>

[![NagleCode](screenshots/naglecode-logo400.png)](https://dannagle.com)
<br>NagleCode ist ein Softwarehersteller und Entwicklungsstudio. 
<br><br><br>


[Möchten Sie, dass Ihr Name/Logo hier aufgeführt wird?](https://github.com/sponsors/dannagle)



<a id="support"></a>
## Unterstützung

* Twitter: [@NagleCode](http://twitter.com/NagleCode)
* Die Foren befinden sich unter: [GitHub-Diskussionen](https://github.com/dannagle/PacketSender/discussions).
* E-Mail: [Paketabsender-Kontakt](https://packetsender.com/contact)
* Verbinden Sie sich mit mir auf [LinkedIn](https://www.linkedin.com/in/dannagle/)

*HINWEIS:* Versuchen Sie, Ihre Firewall (vorübergehend) zu deaktivieren, wenn Sie Probleme unter Windows haben.

<a id="Downloads"></a>
# Herunterladen

## Desktop herunterladen
Offizielle Versionen von Paket Sender können unter [PacketSender.com](http://packetsender.com/download) heruntergeladen werden. Mancherorts wird das Paket Sender weiterverteilt.

![Windows Logo](screenshots/winlogo150.png) ![Mac Logo](screenshots/maclogo150.png) ![Linux Logo](screenshots/Tux150.png)


<!--
Ich bin mir nicht sicher, wann sie wieder verfügbar sein wird.

## Mobile Apps
Die mobilen Editionen von Paket Sender sind vollständig nativ, enthalten nur ein Minimum an Berechtigungen und sammeln keine Daten. Dies ist eine Software, die Sie überwacht. Vielen Dank, dass Sie diese Bemühungen unterstützen.


### Android Mobile App
![Android-Logo](screenshots/android_logo.png)

Die Android-Version finden Sie [auf Google Play](https://play.google.com/store/apps/details?id=com.packetsender.compose) oder [im Amazon Appstore](https://www.amazon.com/dp/B08RXM6KM2/)

[![Paket Sender Android](screenshots/packetsender_android_screenshot.png)](https://play.google.com/store/apps/details?id=com.packetsender.compose

-->


<!--
Ich bin mir nicht sicher, wann sie wieder verfügbar sein wird.

## iOS Mobile App

![Paket Sender Logo](screenshots/ios_logo.png)

Die iOS-Version befindet sich [im Apple App Store](https://apps.apple.com/app/id1558236648#?platform=iphone)

[![Paket Sender iOS](screenshots/packetsender-ios-traffic-log-ascii.png)](https://apps.apple.com/app/id1558236648#?platform=iphone)

-->
<a id="gui"></a>
# GUI 

Paket Sender ist für alle Desktop-Versionen identisch. Der einzige Unterschied besteht darin, dass das Design an das Betriebssystem angepasst ist.

Bildschirmfoto des Paketsenders](screenshots/ps_GUI.png)

1. Einem Paket sind ein Name, eine Zieladresse (Domänennamen werden standardmäßig kurz vor dem Senden aufgelöst), ein Anschluss und Daten zugeordnet.
2. In der Tabelle finden Sie eine Liste der gespeicherten Pakete. Sie können auf Felder in dieser Tabelle doppelklicken, um sie direkt zu bearbeiten.
3. Unten rechts werden UDP-, TCP- und SSL-Serverstatus und Port(s) angezeigt. Wenn Sie darauf klicken, wird das Protokoll aktiviert oder deaktiviert. Der Paketsender unterstützt die Bindung an eine beliebige Anzahl von Ports.
4. Außerdem gibt es eine IP Umschalttaste. Wenn Sie darauf klicken, ändert sich der Wert in IPv4 (Standard), IPv6 oder benutzerdefinierte IP

### Allgemeine Hinweise
* Ein Resend-Wert von "0" bedeutet, dass es sich um ein einmalig gesendetes Paket handelt.
* Während des erneuten Sendens von Paketen gibt es eine Schaltfläche zum Abbrechen aller erneuten Sendungen.
* Bitte überprüfen Sie Ihre Firewall. Windows blockiert aggressiv TCP-basierte Server. Packet Sender funktioniert auch, wenn die Firewall ihn blockiert, kann aber keine unaufgeforderten TCP-basierten Pakete empfangen.
* Eine optionale Antwort kann gesendet werden. Die gleiche Antwort wird für TCP, UDP und SSL verwendet.
* Für den IPv6-Versand benötigen Sie auch die Scope-ID.
* Paket Sender unterstützt gemischte ASCII- und HEX-Schreibweisen:
  * Doppelklicken Sie auf eines der Felder, um den mehrzeiligen Editor aufzurufen.
  * \XX wird in XX in hex übersetzt
  * \n, \r, \t werden in 0A, 0D und 09 übersetzt
  * HEX-Zahlen sind durch Leerzeichen getrennt.
    * Das HEX-Feld versucht, andere gebräuchliche Begrenzungszeichen (wie Kommas, Doppelpunkte (Wireshark), Semikolons, "0x" usw.) zu interpretieren und automatisch zu korrigieren. Sie ist sehr fehlertolerant.
    * Ein einzelner HEX-Stream wird ebenfalls unterstützt. Wenn die Anzahl der Bytes ungerade ist, geht Packet Sender davon aus, dass das erste Byte eine Null sein muss und korrigiert dann automatisch.
  * Beispiel ASCII: hallo welt\r
  * Beispiel HEX: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0d
  * Sie können ein Paket direkt aus dem Verkehrslog speichern. Sie werden aufgefordert, einen Namen einzugeben, und die Quelladresse und der Port werden zu Ihrer Bequemlichkeit umgeschaltet.
  * Sie können eine Datei auch direkt in das HEX-Feld laden. Im HEX-Feld können bis zu 10.922 Bytes gesendet werden. Die theoretische Grenze für das Senden über die Befehlszeile liegt bei 200 MB.


## Hotkeys/Tastaturkürzel

Die Felder im oberen Bereich können mit STRG+1, STRG+2 usw. bis hin zu STRG+8 (Schaltfläche Senden) durchlaufen werden. Auf dem Mac lautet die Tastenkombination Befehl.

Die Hotkeys und Felder sind:
* STRG + 1 = Name
* STRG + 2 = ASCII
* STRG + 3 = HEX
* STRG + 4 = Adresse
* STRG + 5 = Anschluss
* STRG + 6 = Verzögerung beim erneuten Senden
* STRG + 7 = Protokollauswahl
* STRG + 8 = Senden (wird ausgeführt)


Einige Anmerkungen:
* Die Felder sind unabhängig von der Auswahl des Protokolls mit dem entsprechenden Hotkey verknüpft.
* Wenn Sie zur Option TCP/UDP/SSL navigieren, können Sie die Zeichen nach oben/unten oder t/u/s/h verwenden, um eine Auswahl zu treffen.
* Wenn Sie die Arbeit mit Hotkeys automatisieren wollen (z. B. mit [AutoHotKey] (https://www.autohotkey.com/)), sollten Sie die Option "Vorherige Sitzung wiederherstellen" deaktivieren.



<a id="network"></a>
# Netzwerkfunktionen

## IPv4, IPv6, Custom IP

Die integrierten Server von Paket Sender sind so konfiguriert, dass sie entweder IPv4 oder IPv6 unterstützen, aber nicht beide gleichzeitig. Für Clients schalten Paket Sender GUI und CLI beim Senden nahtlos zwischen den beiden Modi um (für IPv6 wird möglicherweise eine Scope-ID benötigt). Klicken Sie unten rechts auf die Umschalttaste IPv4 / IPv6, um zwischen den beiden Optionen zu wechseln.

In den Einstellungen können Sie auch festlegen, dass sich die Server von Packet Sender an eine benutzerdefinierte IP-Adresse binden. Dies kann für Systeme mit mehreren NICs oder komplizierten IP-Konfigurationen sehr nützlich sein. Paket Sender löst einen Fehler aus, wenn er sich an eine Adresse binden soll, die nicht existiert.

![IP-spezifische Bindung](screenshots/ip-spezifische-bindung.png)


<a id="subnetcalculator"></a>
## IPv4-Subnetz-Rechner

Paket Sender hat einen eingebauten Subnetz-Rechner. Es befindet sich unter dem Menü Extras.
![Paket Sender Subnetz Calc](screenshots/packetsender_subnetcalc.PNG)

* Das Log-Fenster (unterer Bereich) zeigt die auf Ihrem Computer gefundenen IPv4- und IPv6-Adressen an, die nicht als Loopback-Adresse fungieren.
* Auf der linken Seite geben Sie die IPv4-Adresse in das Feld IP ein. 
* Auf der linken Seite geben Sie das Subnetz entweder in X.X.X. oder /XX-Notation ein.
* Die Ergebnisse der Berechnung finden Sie auf der rechten Seite.
* Das unten stehende Feld dient der schnellen Überprüfung, ob eine IPv4 in einem Ihrer Subnetze liegt.

## SSL-Klient und -Server

Paket Sender unterstützt den Aufbau verschlüsselter Verbindungen über SSL.
Dies wird sowohl in der grafischen Benutzeroberfläche als auch in der Befehlszeile unterstützt.

Paket Sender bündelt OpenSSL für die Verwendung unter Windows. Auf Mac und Linux verwendet Packet Sender die nativen SSL-Bibliotheken.

Dieses Produkt enthält Software, die vom OpenSSL-Projekt zur Verwendung im OpenSSL-Toolkit entwickelt wurde. (http://www.openssl.org/)_

![Paket Sender Direkt TCP](screenshots/packetsender_ssl.PNG)

SSL-Notizen:
* Die Zertifikatsaushandlung wird sofort während der Verbindung durchgeführt.
* Standardmäßig ignoriert Paket Sender alle SSL-Fehler (abgelaufenes Zertifikat, falscher Hostname, selbstsigniert usw.).
* Paket Sender gibt den Fortschritt der Zertifikatsaushandlung im Verkehrslog aus.
* Paket Sender gibt den im Verkehrslog verwendeten Verschlüsselungsalgorithmus aus (z. B. AES 128).

Paket Sender bündelt ein internes "Snake Oil"-Zertifikat für die Verwendung als Server für Windows. Das Zertifikat und der Schlüssel befinden sich an der gleichen Stelle wie das Paket und die Einstellungen.

_Anmerkung: Das Überschreiben der Zertifizierungsstellen in den Einstellungen überschreibt auch das Schlangenöl-Zertifikat._

Wenn ein SSL-Fehler auftritt, wird dieser von Paketsender in des Verkehrs log, eingetragen. Wenn die Einstellung so ist, dass die Verschlüsselung trotzdem fortgesetzt wird (Standardeinstellung), wird die Verschlüsselung weiter ausgehandelt. Andernfalls wird die Verbindung mit einem Verbindungsabbruch beendet.

![Paket Sender Direkt TCP Abgelaufen](screenshots/packetsender_expired_ssl.png)

## Multicast (experimentell)

Die Multicast-Unterstützung von PaketSender wird durch den Versuch ausgelöst, an eine IPv4-Multicast-Adresse zu senden, oder über das Untermenü Mulitcast. Die Funktion ist derzeit experimentell und hat diese bekannten Probleme.

* Der Paketsender gibt die IPv6-Unterstützung auf, wenn er dem Multicast beitritt.
* Und bleibt verwaist, bis Sie die Einstellungen erneut überprüfen oder versuchen, an IPv6 zu senden.
* Bei WLAN dauert es manchmal 20 Sekunden, bis die Multicast-Verbindung tatsächlich zustande kommt.
* Der Paket-Sender hat keine Logik, um einer Mulitcast-Gruppe wieder beizutreten, wenn der Switch neu gestartet wird oder ein anderer üblicher Fehler auftritt.

Es gibt keine IPv6-Multicast-Unterstützung, obwohl sie auf der Roadmap steht. Sponsoren, die IPv6-Multicast-Unterstützung wünschen, können sich gerne an mich wenden.

<a id="udptraffic"></a>

## UDP-Verkehrsgenerator (experimentell)

Wenn das normale Sendesystem nicht ausreicht, können Sie eine Ziel-IP mit Paketen beschießen, um zu sehen, ob Ihr Gerät damit umgehen kann. Diese finden Sie in der GUI-Symbolleiste unter _Werkzeuge -> Intensiver Verkehrsgenerator_.

Bitte beachten Sie, dass diese Funktion experimentell ist und die angezeigten Metriken noch nicht vollständig getestet wurden. Für einen genaueren Test sollten Sie sich die CLI-Version dieses Werkzeugs ansehen.

![IP-spezifische Bindung](screenshots/udp-traffic-sending.PNG)

# Paket-Sender Merkmale


<a id="cloud"></a>
## Paket-Sender-cloud

Paketsätze können mit dem kostenlosen Dienst [Packet Sender Cloud] (https://cloud.packetsender.com/) schnell gespeichert/abgerufen/geteilt werden. Die Cloud kann auch verwendet werden, um Ihre Pakete öffentlich anzuzeigen und zu verteilen (über eine URL), für die Zusammenarbeit, für Tutorials, für Endbenutzer usw. Paketabsender können öffentliche Paketsätze mit öffentlicher URL importieren.

Hierfür gibt es verschiedene Gründe:

* Halten Sie alle Ihre Pakete bereit, damit Sie sie bei der Installation eines neuen Paketsenders schnell abrufen können.
* Schnelles Wechseln zwischen Paketsätzen bei der Arbeit an verschiedenen Projekten.
* Gemeinsame Nutzung eines Logins (ist erlaubt) für die gemeinschaftliche Erstellung von Paketen
* Eine öffentliche Seite mit Ihren Paketen, die andere schnell finden und importieren können

![Paketsender Cloud importieren](screenshots/cloud-import.png)

Wenn Sie eine Netzwerk-API veröffentlichen, ist es wesentlich einfacher, eine öffentliche Cloud-Seite zu pflegen, als Ihren Nutzern die Pakete mühsam im Detail (IP, Port, Typ usw.) zu beschreiben. Außerdem ist es einfach, diese Seite zu aktualisieren.

Weitere Informationen dazu finden Sie unter
https://cloud.packetsender.com/help


<a id="portable"></a>
## Tragbarer Modus

Paket Sender hat einen "tragbaren" Modus. Beim Programmstart sucht es nach der Datei `portablemode.txt` und füllt alle fehlenden Einstellungsdateien in diesem Laufzeitverzeichnis auf. Diese Dateien sind `packets.ini`, `ps_settings.ini`, `ps.key`, und `ps.pem`.
Sie können auch einige Dateien portabel und die anderen an ihrem Standardspeicherort belassen, indem Sie portablemode.txt entfernen.

### DDLs, die im portablen Modus (nur Konsole) entfernt werden können
Wenn Sie die grafische Benutzeroberfläche nicht benötigen, können Sie diese DDLs entfernen
- Qt5Svg.dll
- libEGL.dll
- libGLESv2.dll
- Qt5Widgets.dll
- Qt5Gui.dll
- opengl32sw.dll
- D3Dcompiler_47.dll
- iconengines-Verzeichnis
- Verzeichnis der imageformats
- Stilverzeichnis

### DDLs, die entfernt werden können, wenn Sie keine sicheren Verbindungen benötigen
Wenn Sie kein SSL benötigen, können Sie diese DDLs entfernen
- libcrypto-1_1-x64.dll
- libssl-1_1-x64.dll


Das Laufzeitverzeichnis für Windows-Benutzer ist derselbe Ort wie die .exe-Datei.

Für MAC-Benutzer befindet sich dieses Laufzeitverzeichnis unter `PacketSender.app/Contents/MacOS`.
Wenn INI-Dateien gefunden werden, werden sie anstelle von `%APPDATEN%` oder `Bibliothek/Anwendungsunterstützung` verwendet.


<a id="smartresponses"></a>
## Intelligente Antworten

Paket Sender unterstützt bis zu 5 intelligente Antworten. 

Um diese Funktion zu aktivieren, wählen Sie _Datei -> Einstellungen_ in der GUI-Symbolleiste. Gehen Sie zur Registerkarte _Smart Responses_ und aktivieren Sie das Kontrollkästchen, **Senden Sie eine intelligente Antwort**.  

![Paket Sender Direkt TCP](screenshots/packetsender_smartreply.PNG)

* Paket Sender vergleicht das Paket mit der von Ihnen gewählten Kodierung.
* Der Paketsender übersetzt die Kodierung, bevor er die Antwort sendet.
* Die verfügbaren Kodierungen sind:
  * Gemischtes ASCII - Die Standardmethode von Packet Sender, ASCII zusammen mit nicht druckbaren Zeichen zu kodieren
  * HEX - normale HEX-Kodierung des Paketsenders


## Makros

Pcket Sender unterstützt diese Makros beim Senden von Antworten:

* {{DATE}} -- Sendet das aktuelle Datum im Format "jjj-mm-tt".
* {{TIME}} -- Sendet die aktuelle Zeit im Format "hh:mm:ss ap".
* {{UNIXTIME}} -- Sendet den aktuellen Epochenzeitstempel.
* {{RANDOM}} -- Sendet eine Zufallszahl, die entweder von 0 bis 32767 oder 2147483647 reicht, je nach 32-Bit oder 64-Bit (das Standard-Installationsprogramm für Windows ist 32-Bit. Mac ist 64-bit).
* {{UNIQUE}} -- Sendet eine zufällige Zeichenkette. Verwendet eine interne UUID, um sie zu erzeugen.
Der Paketsender tauscht das Makro vor dem Senden mit echten Werten aus.


<a id="persistent"></a>
## Persistentes TCP und SSL

Paket Sender unterstützt dauerhafte TCP- und SSL-Verbindungen über ein separates GUI-Fenster. Sie wird über ein Kontrollkästchen im Hauptfenster oder über das Fenster Einstellungen aktiviert.

![Paketsender Direkt TCP und SSL](screenshots/packetsender_direct_tcp.PNG)

### Anmerkungen zu Persistente TCP und SSL:
* Es können beliebig viele dauerhafte Verbindungen hergestellt werden.
* Zuvor gespeicherte Pakete können in der Dropdown-Liste geladen werden.
* Es gibt eine "Raw"-Ansicht und eine "ASCII"-Ansicht. Die ASCII-Ansicht ist nützlich für die Fehlersuche bei Daten, die von der Rohansicht nicht ausgegeben werden.
* Der Verkehr wird auch im Verkehrs log des Hauptfensters gespeichert.
* Eine Datei kann auf die dauerhafte Verbindung hochgeladen werden. Möglicherweise möchten Sie die Logging-Funktion deaktivieren, wenn Sie diese Funktion verwenden.
* Der Timer unten links startet, sobald ein gültiges Datenpaket gesendet/empfangen wird. Sie endet, wenn die Verbindung beendet wird.
* Sie können optional einen Wagenrücklauf anhängen, wenn Sie eine Schnellübertragung durchführen, indem Sie die Return-Taste drücken. Dies ist nützlich für Eingabeaufforderungsmenüs über TCP/SSL-Verbindungen. Paket Sender merkt sich den vorherigen Zustand des Kontrollkästchens \r.
* Bei eingehenden dauerhaften Verbindungen zum Server wird das separate GUI-Fenster geöffnet.
* Beim erneuten Senden wird das dauerhafte Verbindungspaket in das neue GUI-Fenster übertragen. Wenn Sie auf "Erneut senden(1)" klicken, wird der Vorgang abgebrochen.

Dauerhafte Verbindungen werden nicht über die Befehlszeile unterstützt.


<a id="http"></a>
# HTTP/HTTPS POST & GET
Paket Sender unterstützt das Senden von POST/GET-Anfragen über HTTP und HTTPS. 
Das Dropdown-Menü Protokoll enthält die folgenden Optionen: HTTP GET, HTTP POST, HTTPS GET, HTTPS POST. Wenn Sie HTTP(S) auswählen, werden die Eingabefelder auf aktualisiert: Name, Anfrage, Adresse, Daten (wenn POST ausgewählt ist), Schaltfläche Daten generieren (wenn POST ausgewählt ist), Datei laden (wenn POST ausgewählt ist). 

## Senden von HTTP/HTTPS GET/POST-Anfragen
![](/screenshots/ps_http_getfields.PNG)
* Wählen Sie HTTP(S) GET oder POST aus dem Dropdown-Menü für das Protokoll
* Geben Sie in das Feld *Adresse* die Domäne oder IP ein.
* Im Feld *Anfrage* fügen Sie den URL-Pfad hinzu, falls erforderlich
* Im Feld *Port* ist der Standardwert für HTTP 80 und für HTTPS 443.
* Aktivieren Sie *Persistent TCP*, um die Serverdaten deutlicher zu sehen (HTTP-Header werden automatisch entfernt). 

**Sie können auch eine vollständige URL in das Anforderungsfeld einfügen, und Packet Sender analysiert und füllt die anderen Felder automatisch aus.**

### Für POST-Anfragen:
* Sie können die Daten manuell in das Feld *Daten* eintragen.
	* Das Format würde lauten: Schlüssel=Wert
	* Bei mehreren Parametern: key=value&key=value&key=value
* Oder Sie können auf die Schaltfläche *Daten generieren* klicken.

<img src="/screenshots/ps_http_datagenerator.PNG" width="400" height="284">

* Um Daten hinzuzufügen, geben Sie die Parameter Schlüssel und Wert ein. Klicken Sie auf die Schaltfläche **+**. 
* Mit der Schaltfläche "+" können Sie mehrere Parameter hinzufügen. 
* Parameter entfernen, indem Sie auf die Schaltfläche X neben dem Parameter klicken
* Sobald Sie die Parameter hinzugefügt haben, klicken Sie auf Ok und die Daten werden im Feld Daten generiert. 

### Um Authentifizierungsdaten hinzuzufügen:

<img src="/screenshots/ps_http_authgenerator.PNG" width="800" height="339">

* Gehen Sie zu Datei -> Einstellungen -> HTTP
* Markieren Sie *Auth Header generieren*.
* Eingabe von *Host*, *UN/Client ID* und *PW/Access*.
* Klicken Sie auf *HTTP Auth Header*, um den Authentifizierungs-Header zu generieren

<a id="panelgen"></a>
# Panel Generator
Paket Sender unterstützt die Erstellung von Bedienfeldern. Felder bestehen aus Schaltflächen, denen Skripte (Pakete) zugewiesen sind. Wenn Sie auf die Schaltfläche klicken, werden die Pakete, auf die die Schaltfläche verweist, ausgeführt. 

<img src="/screenshots/ps_panel_1.PNG" width="400" height="358">

## Laden eines Panels
Panels können auf zwei Arten erstellt werden:
* Klicken Sie auf **Panels** in der Symbolleiste und wählen Sie entweder Starterpanel laden oder Leeres Panel-Projekt
	* Starter-Panel laden lädt das als Starter zugewiesene Panel. Wenn kein Panel als Startpunkt festgelegt ist, wird ein leeres Panel-Projekt geöffnet. 
* Markieren von 2 oder mehr gespeicherten Paketen und Klicken auf die Schaltfläche **Panel generieren** (Die Schaltfläche "Panel generieren" erscheint nur, wenn mehrere Pakete markiert sind) 

![](/screenshots/ps_panel_generate.PNG)


## Skripting eines Panels
Um mit der Skripterstellung für die Schaltflächen auf Ihrem Bedienfeld zu beginnen, müssen Sie ein Bedienfeld öffnen und zum Bearbeitungsbildschirm wechseln. Sobald ein Panel-Projekt geöffnet ist, prüfen Sie die Schaltfläche in der unteren rechten Ecke. Wenn auf dieser Schaltfläche "Betrachten" steht, befinden Sie sich auf dem Bildschirm "Betrachten". Klicken Sie auf die Schaltfläche, um das Panel in den Bearbeitungsbildschirm zu verschieben. 

Auf dem Bearbeitungsbildschirm können Schaltflächen und Skripte zum Bedienfeld hinzugefügt werden. 

### Schaltfläche Scripting
Die Schaltflächenskripte enthalten den Namen des zu sendenden Pakets. 

<img src="/screenshots/ps_panel_2.PNG" width="400" height="360">

Mehrere Pakete können auf eine Schaltfläche gelegt werden, indem jeder Name in eine neue Zeile eingefügt wird.

<img src="/screenshots/ps_panel_5.PNG" width="400" height="358">

Der Panel Generator unterstützt das Hinzufügen einer Verzögerung zwischen mehreren Paketen durch das Hinzufügen von "delay:_# of seconds_" zwischen den Paketen.

<img src="/screenshots/ps_panel_4.PNG" width="400" height="359">

Panel Generator unterstützt das Hinzufügen eines Skripts zum Laden eines neuen Panels durch Hinzufügen von "panel:_panel id #_". Sobald alle vorherigen Skripte auf der Schaltfläche ausgeführt wurden, wechselt das Panel zum nächsten Panel. 

<img src="/screenshots/ps_panel_8.PNG" width="400" height="358">


### Hinzufügen von Dateien/URLs
Panel Generator unterstützt das Hinzufügen von Schaltflächen, die auf lokal gespeicherte Dateien oder URLs verweisen. 
Datei-/URL-Schaltflächen können im Bearbeitungsbildschirm durch Klicken auf das *+* in der unteren rechten Ecke hinzugefügt werden. 
* Für Dateien: Gehen Sie zu der Datei auf dem PC, klicken Sie mit der rechten Maustaste auf die Datei und wählen Sie Kopieren. Fügen Sie dies in das Textfeld _URL oder Datei_ in Packet Sender ein
* Für URLs: Kopieren Sie die URL in das Textfeld _URL oder Datei_ in Packet Sender
	* URLs müssen mit http:// oder https:// beginnen.


![](/screenshots/ps_panel_7.PNG) 

Sobald die Datei oder URL kopiert ist, werden Sie aufgefordert, einen Namen für die Schaltfläche einzugeben. Die Schaltflächen werden am unteren Rand des Fensters eingeblendet. 

Wenn Sie auf dem Bearbeitungsbildschirm auf diese Schaltflächen klicken, können Sie den Datei-/URL-Link und den Namen der Schaltfläche bearbeiten. Sie können die Schaltfläche auch löschen, indem Sie auf das **X** im Popup-Fenster klicken. 

![](/screenshots/ps_http_changeURL.PNG)

Wenn Sie auf dem Ansichtsbildschirm auf diese Schaltflächen klicken, wird die URL im Standardbrowser aufgerufen oder die Datei geöffnet (mit dem Standardprogramm für den Dateityp). 


![](/screenshots/ps_panel_6.PNG) 

### Panel Bearbeiten/Speichern
Wenn Sie sich im Bearbeitungsbildschirm eines Panels befinden, gibt es eine Symbolleiste mit den Menüs Datei, exportieren, Einstellungen und Hilfe. Von dieser Symbolleiste aus können Sie Panel-Projekte speichern, exportieren, importieren, laden und das aktuelle Panel-Projekt bearbeiten. 

In den Einstellungen können Sie Folgendes tun:
* Panel-Name festlegen - wählen Sie diese Option, um das aktuelle Panel-Projekt umzubenennen.
* Panel-ID festlegen - wählen Sie diese Option, um die mit dem aktuellen Panel-Projekt verbundene ID zu ändern.
_Anmerkung: Das Einstellen einer verwendeten ID ersetzt dieses Panel_
* Starterpanel - wählen Sie diese Option, um das aktuelle Panelprojekt als Starterpanel festzulegen. 
* Panel löschen - Damit wird ein Menü mit den aktuellen Panel-Projekten angezeigt. Wählen Sie ein Panel-Projekt aus, um es zu löschen. Hinweis: Die Schaltflächen und Skripte bleiben auf dem Bearbeitungsbildschirm erhalten, bis das Panel geschlossen wird.



<a id="cli"></a>
# Kommandozeile
Paket Sender kann über die Befehlszeile auf Ihrem Computer verwendet werden. 

Für Windows verwenden Sie die Erweiterung .com (`packetsender.com`), um die Befehlszeilenschnittstelle zu verwenden. Optional können Sie auch `Paketsender` ohne Erweiterung verwenden. Mit der Endung .exe wird die GUI gestartet. 


![Paket Sender CLI Bildschirmfoto](screenshots/packetsender_command_line.png)

Für Linux folgt das Kommandozeilensystem in Packet Sender demselben Muster wie andere Linux-Dienstprogramme. Sie hat einen langen Namen (z. B. --version) und einen kurzen Namen (z. B. -v). Diese Optionen können in beliebiger Reihenfolge angeordnet werden und werden von Packet Sender korrekt ausgewertet. Die letzten 3 Optionen sind positionsabhängig und müssen als letztes erscheinen. Diese sind IP, Port und Daten. Diese letzten Optionen sind optional, wenn ein gespeichertes Paket verwendet wird.

```text
paket sender --help
Paket Sender ist ein Netzwerk-UDP/TCP/SSL/HTTP-Testdienstprogramm von NagleCode
Siehe https://PacketSender.com/ für weitere Informationen.

Optionen:
  -?, -h, --help Zeigt die Hilfe zu den Befehlszeilenoptionen an.
  --help-all Zeigt die Hilfe einschließlich Qt-spezifischer Optionen an.
  -v, --version Zeigt Versionsinformationen an.
  -q, --quiet Leiser Modus. Nur empfangene Daten ausgeben.
  -x, --hex Zu sendende Daten als Hexadezimalwert parsen (Standard für
                            TCP/UDP/SSL).
  -a, --ascii Parsen der zu sendenden Daten als gemischtes ASCII (Standard für http
                            und GUI).
  -A, --ASCII Parsen der zu sendenden Daten als reine ASCII-Daten (keine \xx)
                            Übersetzung).
  -w, --wait <ms> Warten Sie bis zu <Millisekunden> auf eine Antwort nach
                            senden. Null bedeutet, dass nicht gewartet werden soll (Standard).
  -f, --file <Pfad> Den Inhalt des angegebenen Pfades senden. Maximal 10 MiB für
                            UDP, 100 MiB für TCP/SSL.
  -b, --bind <Port> Port binden. Standardwert ist 0 (dynamisch).
  -6, --ipv6 Erzwingt IPv6. Dasselbe wie -B "::". Standard ist IP:Any.
  -4, --ipv4 Erzwingt IPv4.  Dasselbe wie -B "0.0.0.0". Standard ist
                            IP:Beliebig.
  -B, --bindip <IP> Benutzerdefinierte IP binden. Standard ist IP:Any.
  -t, --tcp TCP senden (Standard).
  -s, --ssl SSL senden und Fehler ignorieren.
  -S, --SSL SSL senden und bei Fehlern anhalten.
  -u, --udp UDP senden.
  --http <http> HTTP senden. Erlaubte Werte sind GET (Standard) und
                            POST
  -n, --name <name> Ein zuvor gespeichertes Paket mit dem Namen <name> senden. Andere
                            setzt gespeicherte Paketparameter außer Kraft.
  --bps <bps> Intensiver Verkehr. Berechnen Sie den Satz auf der Grundlage des Wertes von
                            Bits pro Sekunde.
  --num <Nummer> Intensiver Verkehr. Anzahl der zu sendenden Pakete. Standard
                            unbegrenzt.
  --rate <Hertz> Intensiver Verkehr. Bewerten. Die Option in bps wird ignoriert.
  --usdelay <microseconds> Intensiver Verkehr. Verzögerung beim erneuten Senden. Wird verwendet, wenn die Rate 0 ist.
                            Die Option in bps wird ignoriert.

Argumente:
  Adresse Zieladresse/URL. Fakultativ für gespeicherte Pakete.
  port Zielhafen/POST-Daten. Fakultativ für gespeicherte
                            Päckchen.
  daten Zu sendende Daten. Fakultativ für gespeicherte Pakete.
```
## Beispiel CLI
Die Befehlszeilenschnittstelle hat unter Windows, Linux und MAC das gleiche Format. 

Das Format ist: `Paketsender [Optionen] Adresse Port Daten`

```text
packetsender -taw 500 mirrors.xmission.com 21 "USER anonymous\r\nPASS chrome@example.com\r\n"
TCP (65505)://mirrors.xmission.com:21 55 53 45 52 20 61 6e 6f 6e 79 6d 6f 75 73 0d 0a 50 41 53 53 20 63 68 72 6f 6d 65 40 65 78 61 6d 70 6c 65 2e 63 6f 6d 0d 0a
Antwortzeit:5:51:37.042 pm
Antwort HEX:32 32 30 2D 57 65 6C 63 6F 6D 65 20...
Antwort ASCII:220-Willkommen bei XMission Internet...
```

## Beispiele für die Bindung an Port und benutzerdefinierte IP, IPv4 oder IPv6

Die Kommandozeile von Packet Sender kann an benutzerdefinierte Ports gebunden werden, um IPv4/6-Modi oder mehrere NICs mit der Option -B zu erzwingen.
```text
packetsender -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hello\r"
packetsender -taw 3000 192.168.0.201 5005 "Hello\r"
packetsender -B 192.168.0.200 -taw 3000 192.168.0.201 5005 "Hello\r"
packetsender -B fe80::a437:399a:3091:266a%ethernet_32769 -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hello\r"
packetsender -B fe80::a437:399a:3091:266a -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hello\r"
```

## Beispiel CLI mit SSL und Ignorieren von Fehlern

Die Befehlszeile bietet die Möglichkeit, SSL-Fehler zu ignorieren oder abzubrechen. Die Standardeinstellung ist "ignorieren".

* Verwenden Sie die Option -s, um SSL zu senden und Fehler zu ignorieren.
* Option -S verwenden, um SSL zu senden und bei Fehlern anzuhalten

```text
packetsender -saw 500 expired.packetsender.com 443 "GET / HTTP/1.0\r\n\r\n"
SSL-Fehler: Das Zertifikat ist abgelaufen
SSL (54202)://expired.packetsender.com:443 47 45 54 20 2f 20 48 54 54 50 2f 31 2e 30 0d 0a 0d 0a
Chiffre: Verschlüsselt mit AES(128)

Antwortzeit:3:24:55.695 pm
Antwort HEX:48 54 54 50 2f 31 2e 31 20 34 32 31 20 0d 0a 53 65 72 76 65 72 3a 20 6e 67 69 6e 78 2f 31 2e 31 30 2e 30 20 28 55 62 75 6e 74 75 29 0d
Antwort ASCII:HTTP/1.1 421 \r\nServer: nginx/1.10.0 (Ubuntu)\r
```

## Beispiel CLI mit HTTP
Beachten Sie, dass dabei die eingebauten Standardpakete verwendet werden.
```text
packetsender --name "HTTPS POST Params"
packetsender --http GET "https://httpbin.org/get"
packetsender --http POST "https://httpbin.org/post" "{}"
```



<a id="cliintensetraffic"></a>

## Beispiel mit CLI Intensiver Verkehrsgenerator
Der befehlsintensive Verkehrsgenerator funktioniert ähnlich wie die GUI-Version, ist aber etwas genauer und bietet mehr Steuerungsmöglichkeiten (und mehr Intensität!).

Im Folgenden finden Sie Beispiele für die Verwendung dieser Funktion. Beachten Sie, dass diese Berechnungen nach bestem Wissen und Gewissen erfolgen. Das funktioniert ganz gut, aber Prozessorspitzen oder verschiedene Netzwerkprobleme können das System stören. Die Einfädelung erfolgt nicht in Echtzeit und ist auch nicht besonders intelligent in ihren Kompensationsversuchen.

* erneut senden "Mein Wahnsinns-Paket" mit einer Rate von 20 Hz 
* erneut senden "Mein Wahnsinns-Paket" erneut mit 2000 Baud pro Sekunde
* erneut senden "Mein Wahnsinns-Paket" so schnell wie möglich zurück
* erneut senden "Mein Wahnsinns-Paket" mit 2000000 Mikrosekunden Verzögerung zwischen den einzelnen Paketen erneut senden

**Anmerkung: Für Windows verwenden Sie die ".com"-Erstellung, d. h. jedes Beispiel wäre packetsender.com**

```text
packetsender --rate 20 --name "Mein Wahnsinns-Paket"
packetsender --bps 2000 --name "Mein Wahnsinns-Paket"
packetsender --rate 0 --name "Mein Wahnsinns-Paket"
packetsender --usdelay 2000000 --name "Mein Wahnsinns-Paket"
```


<a id="building"></a>
# Gebäude Paket Sender
Die einzige Abhängigkeit ist Qt SDK

## Paketsender für Windows/MAC erstellen
1. Laden Sie das Qt-Installationsprogramm von http://www.qt.io/download-open-source/ herunter.
1. Lassen Sie MingGW installieren, wenn Sie keinen Compiler haben.
1. Öffnen Sie das Projekt PacketSender.pro
1. Bauen! 

Die Windows- und Mac-Versionen wurden mit Qt 5.12 erstellt. Paket Sender unterstützt zwar Qt 6, aber nicht
cmake unterstützen.

## Paket Sender für Linux erstellen
Hier ist die Reihenfolge der Befehle für Ubuntu 16.04. Bitte passen Sie es an Ihre Linux-Plattform an. Paket Sender benötigt keine zusätzlichen Bibliotheken neben dem Qt SDK. Mir wurde gesagt, dass es Probleme bei der Erstellung der Fedora-Version gibt. Wenn ein Fedora-Experte einen Einblick hat, lassen Sie es mich bitte wissen, und ich werde Ihre Anweisungen hinzufügen.

Wenn Sie abenteuerlustig sind, können Sie auch aus dem Master-Zweig bauen. Sie enthält die neueste stabile Version. Der Entwicklungszweig sollte wahrscheinlich vermieden werden.

```bash
sudo apt-get update
sudo apt-get install qt5-default build-essential
wget https://github.com/dannagle/PacketSender/archive/(Version).tar.gz
tar -xzvf (Version).tar.gz
cd PacketSender-(Version)/src
qmake PacketSender.pro
make
```

Zum Ausführen verwenden:
```text
./PacketSender
```

Wenn es nicht ausgeführt wird, müssen Sie es möglicherweise als ausführbar festlegen
```text
chmod a+x PacketSender
```


# Verbesserungen/Forderungen

Vermissen Sie eine Funktion? Sie können [mich beauftragen, es zu Paket Sender hinzuzufügen] (https://packetsender.com/enhancements).

# Rechtliches / Compliance

Die Lizenz ist GPL v2 oder später. [Kontaktieren Sie mich](https://packetsender.com/contact), wenn Sie eine andere Lizenz benötigen.
Einige Distributionen von Paket Sender verwenden möglicherweise [OpenSSL](https://www.openssl.org/).
Die aktuellste VPAT [finden Sie](vpat_2.4_packetsender.pdf) in diesem Repo.


# Urheberrecht

Paket Sender wurde von [Dan Nagle](https://dannagle.com/) geschrieben und wird veröffentlicht von &copy; NagleCode, LLC - [@NagleCode](https://twitter.com/NagleCode) - [PacketSender.com](https://packetsender.com)
