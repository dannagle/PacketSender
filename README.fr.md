# Introduction

![Logo Packet Sender ](screenshots/packetsender_banner.png)


*Lire ceci dans d’autres langues : [English](README.md), [Español](README.es.md), [Deutsch](README.de.md), [Français](README.fr.md).*


[![Téléchargements](https://img.shields.io/github/downloads/dannagle/PacketSender/total.svg)](https://packetsender.com/download)


Packet Sender est un utilitaire open source permettant d’envoyer et de recevoir des paquets TCP, UDP et SSL (TCP cryptés) ainsi que des requêtes HTTP / HTTPS et la génération de panneaux. La branche principale supporte officiellement Windows, Mac et Desktop Linux (avec Qt). D’autres endroits peuvent recompiler et redistribuer Packet Sender. Packet Sender est gratuit et sous licence GPL v2 ou ultérieure. Il peut être utilisé à des fins commerciales et personnelles. Si vous trouvez l’application utile, veuillez envisager de faire un don / parrainer afin que le développement puisse continuer.



# Table des matières
* [Auteurs](#sponsors)
  * Visitez [IWL.com](https://www.iwl.com/)v
  * Visite [NagleCode.com](https://dannagle.com/)

* [GUI](#gui)
* [Générateur de trafic intense (GUI)](#udptraffic)
* [Réseau](#network)
* [Téléchargements](#downloads)
* [Soutien](#support)
* [Calculateur de sous-réseau IPv4](#subnetcalculator)
* [Packet Sender Cloud](#cloud)
* [Mode portable](#portable)
* [Macros et réponses intelligentes](#smartresponses)
* [TCP et SSL persistants](#persistent)
* [Requêtes HTTP/HTTPS](#http)
* [Générateur de panneaux](#panelgen)
* [Interface de ligne de commande](#cli)
* [Générateur de trafic intense (CLI)](#cliintensetraffic)
* [Création de Packet sender](#building)




<a id="sponsors"></a>
## Auteurs

Packet Sender tient à remercier les sponsors suivants.

[![IWL](screenshots/iwl_logo.png)](https://www.iwl.com/)
<br>IWL est une société californienne qui crée des produits de réseaux informatiques.
<br><br><br>

[![NagleCode](screenshots/naglecode-logo400.png)](https://dannagle.com)
<br>NagleCode est un éditeur de logiciels et un studio de développement.
<br><br><br>


[Voulez-vous que votre nom / logo figure ici?](https://github.com/sponsors/dannagle)



<a id="support"></a>
## Soutien

* Twitter: [@NagleCode](http://twitter.com/NagleCode)
* Les forums sont à : [GitHub Discussions](https://github.com/dannagle/PacketSender/discussions).
* Email:[Contact expéditeur du paquet](https://packetsender.com/contact)
* Connectez-vous avec moi sur [LinkedIn](https://www.linkedin.com/in/dannagle/)

*REMARQUE:* Essayez (temporairement) de désactiver votre pare-feu si vous rencontrez des problèmes dans Windows.

<a id="downloads"></a>
# Téléchargements

## Téléchargement du Version Bureau
Les versions officielles de Packet Sender peuvent être téléchargées à l’adresse [PacketSender.com](http://packetsender.com/download). Certains endroits redistribuent Packet Sender.

![Logo Windows](screenshots/winlogo150.png) ![Logo Mac](screenshots/maclogo150.png) ![Logo Linux](screenshots/Tux150.png)


<!--
Je ne sais pas quand cela reviendra.

## Applications mobiles
Les éditions mobiles de Packet Sender sont entièrement natives, contiennent des autorisations minimales et ne collectent aucune donnée. C’est un logiciel qui vous respsecte. Merci de soutenir cet effort.


### Application mobile Androïde
![Logo Androïde](screenshots/android_logo.png)

La version Androïde se trouve [sur Google Play](https://play.google.com/store/apps/details?id=com.packetsender.compose) ou [sur Amazon Appstore](https://www.amazon.com/dp/B08RXM6KM2/)

[![Packet Sender Androïde](screenshots/packetsender_android_screenshot.png)](https://play.google.com/store/apps/details?id=com.packetsender.compose

-->


<!--
Je ne sais pas quand cela reviendra.

## Application mobile iOS

![logo Packet Sender ](screenshots/ios_logo.png)

La version iOS se trouve [sur l’App Store d’Apple](https://apps.apple.com/app/id1558236648#?platform=iphone)

[![Packet Sender iOS](screenshots/packetsender-ios-traffic-log-ascii.png)](https://apps.apple.com/app/id1558236648#?platform=iphone)

-->
<a id="gui"></a>
# GUI

Packet Sender est identique pour toutes les versions de bureau. La seule différence est son thème pour correspondre au système d’exploitation.

![capture d'écran Packet Sender](screenshots/ps_GUI.png)

1. Un paquet a un nom, une adresse de destination (les noms de domaine sont résolus par défaut juste avant l’envoi), un port et des données qui lui sont associées.
2. Dans le tableau, il y a une liste de paquets enregistrés. Vous pouvez double-cliquer pour modifier directement les domaine de ce tableau.
3. En bas à droite, il y a l’état et le(s) port(s) du serveur UDP, TCP et SSL. Cliquez dessus pour activer ou désactiver le protocole. Packet Sender prend en charge la liaison à n’importe quel nombre de ports.
4. Il y a aussi un bouton bascule IP. En cliquant dessus, vous le changez en IPv4 (par défaut), IPv6 ou IP personnalisée

### Notes générales
* Une valeur de renvoi de "0" signifie qu'il s'agit d'un paquet unique.
* Pendant le renvoi des paquets, il y aura un bouton pour annuler tous les renvois.
* Veuillez vérifier votre pare-feu. Windows bloque agressivement les serveurs TCP. Packet Sender fonctionnera toujours si le pare-feu le bloque, mais il ne peut pas recevoir de paquets TCP non sollicités.
* Une réponse facultative peut être envoyée. La même réponse est utilisée pour TCP, UDP et SSL.
* Pour l’envoi IPv6, vous aurez également besoin de l’ID d’étendue.
* Packet Sender prend en charge la notation mixte ASCII et HEX:
  * Double-cliquez sur l’un des champs pour afficher l’éditeur multiligne
  * \XX est traduit en XX en hexadécimal
  * \n, \r, \t seront traduits en 0A, 0D et 09
  * Les numéros HEX sont délimités par des espaces
    * Le champ HEX tentera d’interpréter d’autres délimiteurs courants (tels que les virgules, les deux-points (Wireshark), les points-virgules, " 0x « , etc.) et de corriger automatiquement. Il est très tolérant aux fautes.
    * Un seul flux HEX est également pris en charge. Si le nombre d’octets est impair, Packet Sender supposera que l’octet avant a besoin d’un zéro, puis corrigera automatiquement.
  * Exemple ASCII: salut tout le mondes\r
  * Exemple HEX: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0d
  * Vous pouvez enregistrer un paquet directement à partir du journal de trafic. Vous serez invité à entrer un nom, et l’adresse source et le port seront commutés pour votre commodité.
  * Vous pouvez également charger un fichier directement dans le champ HEX. Le domaine HEX prend en charge l’envoi de 10 922 octets. La limite théorique pour l’envoi via la ligne de commande est de 200 Mo.


## Raccourcis clavier/raccourcis clavier

Les domaines situés en haut de l'écran peuvent être parcourus en utilisant CTRL+1, CTRL+2, etc., jusqu'à CTRL+8 (bouton d'envoi). Sur Mac, la touche de raccourci est Commande.

Les raccourcis clavier et les champs sont les suivants :
* CTRL + 1 = Nom
* CTRL + 2 = ASCII
* CTRL + 3 = HEX
* CTRL + 4 = Adresse
* CTRL + 5 = Port
* CTRL + 6 = Délai de renvoi
* CTRL + 7 = Sélection du protocole
* CTRL + 8 = Envoyer (s’exécute)


Quelques remarques :
* Les domaines sont liés à la touche de raccourci correspondante, indépendamment de la sélection du protocole.
* Lorsque vous accédez à l’option TCP/UDP/SSL, vous pouvez utiliser des caractères haut/bas ou t/u/s/h pour effectuer une sélection.
* Si vous envisagez d’automatiser avec des raccourcis clavier (à l’aide d’outils tels que [AutoHotKey](https://www.autohotkey.com/)), vous pouvez désactiver « Restaurer la session précédente ».



<a id="network"></a>
# Fonctionnalités réseau

## IPv4, IPv6, IP personnalisée

Les serveurs intégrés de Packet Sender sont configurés pour prendre en charge IPv4 ou IPv6, mais pas les deux en même temps. Pour les clients, l’interface graphique de l’expéditeur de paquets et l’interface de ligne de commande basculeront de manière transparente entre les deux modes lors de l’envoi (l’ID de portée peut être nécessaire pour IPv6). Cliquez sur la bascule IPv4 / IPv6 en bas à droite pour basculer entre les deux.

Dans les paramètres, vous pouvez également forcer les serveurs de Packet Sender à se lier à une adresse IP personnalisée. Cela peut être très utile pour les systèmes avec plusieurs cartes réseau ou des configurations IP compliquées. Packet Sender déclenchera une erreur s’il lui est demandé de se lier à une adresse qui n’existe pas.

![Liaison spécifique à l’IP](screenshots/ip-specific-binding.png)


<a id="subnetcalculator"></a>
## Calculateur de sous-réseau IPv4

Packet Sender dispose d’un calculateur de sous-réseau intégré. C’est sous le menu Outils.
![Calculateur du sous-réseau de Packet Sender](screenshots/packetsender_subnetcalc.PNG)

* La fenêtre de journal (section inférieure) affichera les adresses IPv4 et IPv6 sans bouclage trouvées sur votre ordinateur.
* Sur le côté gauche, entrez l’adresse IPv4 dans le domaine IP.
* Sur le côté gauche, entrez le sous-réseau en notation X.X.X. ou /XX
* Les résultats du calcul sont à droite.
* Le champ ci-dessous est une vérification rapide pour voir si un IPv4 se trouve dans l’un de vos sous-réseaux.

## Client et serveur SSL

Packet Sender prend en charge l’établissement de connexions chiffrées via SSL.
Ceci est pris en charge dans l’interface graphique et sur la ligne de commande.

Packet Sender regroupe OpenSSL pour une utilisation dans Windows. Sur Mac et Linux, Packet Sender utilisera les bibliothèques SSL natives.

_Ce produit contient un logiciel développé par le projet OpenSSL pour être utilisé dans la boîte à outils OpenSSL. (http://www.openssl.org/)_

![TCP direct de Packet Sender](screenshots/packetsender_ssl.PNG)

Notes SSL :
* La négociation du certificat est traitée immédiatement lors de la connexion.
* Par défaut, Packet Sender ignore toutes les erreurs SSL (certificat expiré, nom d’hôte incorrect, auto-signé, etc.).
* Packet Sender affiche la progression de la négociation du certificat dans le journal de trafic.
* Packet Sender génère l’algorithme de chiffrement utilisé dans le journal de trafic (tel que AES 128).

Packet Sender propose un certificat interne "Snake Oil" pour une utilisation en tant que serveur pour Windows. Le certificat et la clé se trouvent au même endroit que le paquet et les paramètres.

_Note: Le remplacement des emplacements de certificat dans Paramètres remplace également le certificate de snake oil._

S’il y a une erreur SSL, Packet Sender la transmettra dans le journal de trafic. Si le paramètre est de continuer de toute façon (par défaut), il continuera à négocier le cryptage. Sinon, la connexion se termine par l’échec de la connexion.

![TCP direct de Packet Sender a expiré](screenshots/packetsender_expired_ssl.png)

## Multicast(expérimentale)

La prise en charge de la multicast de Packet Sender est déclenchée en tentant d’envoyer vers une adresse de multidiffusion IPv4 ou à partir du sous-menu multidiffusion. La fonctionnalité est actuellement expérimentale et présente ces problèmes connus.

* Packet Sender abandonne la prise en charge d’IPv6 lors de la multicast.
* Et reste abandonné jusqu’à ce que vous reveniez sur les paramètres ou tentiez d’envoyer vers IPv6
* Sur le wifi, il faut parfois 20 secondes pour que la jointure multicast prenne réellement effet.
* Packet Sender n’a aucune logique pour rejoindre un groupe multicast si le commutateur redémarre ou une autre erreur courante.

Il n’y a pas de support de multicast IPv6, bien qu’il soit sur la feuille de route. Les sponsors souhaitant un support multicast IPv6 sont invités à me contacter.

<a id="udptraffic"></a>

## Générateur de trafic UDP (expérimental)

Pour quand le système d’envoi normal ne suffit pas, vous pouvez marteler une adresse IP cible avec des paquets pour voir si votre appareil peut le gérer. Vous pouvez le trouver dans la barre d’outils de l’interface graphique à l'_outils -> Générateur de trafic intense_

Veuillez noter que cette fonctionnalité est expérimentale et que les métriques affichées n’ont pas été entièrement testées. Pour un test plus précis, vous pouvez consulter la version CLI de cet outil.

![Liaison spécifique à l’IP](screenshots/udp-traffic-sending.PNG)

# Fonctionnalités de l’expéditeur de paquets


<a id="cloud"></a>
## Cloud Packet Sender

Les ensembles de paquets peuvent être rapidement enregistrés/récupérés/partagés à l’aide du service gratuit [Cloud Packet Sender ](https://cloud.packetsender.com/). Le cloud peut également être utilisé pour afficher et distribuer publiquement vos paquets (via une URL) pour la collaboration, les tutoriels, les utilisateurs finaux, etc. Packet Sender peut importer des ensembles de paquets publics avec une URL publique.

Il y a plusieurs raisons de le faire:

* Garder tous vos paquets prêts afin que vous puissiez les récupérer rapidement lors de l’installation d’un nouvel version de Packet Sender
* Basculer rapidement entre les ensembles de paquets lorsque vous travaillez sur différents projets.
* Partage d’un login (c’est autorisé) pour la génération collaborative de paquets
* Avoir une page publique de vos ensembles de paquets afin que les autres puissent rapidement trouver et importer

![Importation du cloud de Packet Sender](screenshots/cloud-import.png)

Si vous publiez une API réseau, la maintenance d’une page de cloud public est beaucoup plus facile que de détailler péniblement (IP, port, type, etc.) les paquets à vos utilisateurs. De plus, la mise à jour de cette page est facile.

Plus d’informations à ce sujet peuvent être trouvées à l’adresse
https://cloud.packetsender.com/help


<a id="portable"></a>
## Mode portable

Packet Sender a un mode « portable ». Au lancement, il recherchera 'portablemode.txt' et remplira tous les fichiers de paramètres manquants dans ce répertoire d’exécution. Ces fichiers sont 'packets.ini', 'ps_settings.ini', 'ps.key' et 'ps.pem'.
Vous pouvez également avoir certains fichiers portables et l’autre dans leur emplacement standard en supprimant portablemode.txt.

### DDL pouvant être supprimés en mode portable de la console uniquement
Si vous n’avez pas besoin de l’interface graphique, vous pouvez supprimer ces DDL
- Qt5Svg.dll
- libEGL.dll
- libGLESv2.dll
- Qt5Widgets.dll
- Qt5Gui.dll
- opengl32sw.dll
- D3Dcompiler_47.dll
- Répertoire des moteurs d'icônes
- répertoire des formats d'image
- répertoire des styles

### DDL qui peuvent être supprimés si vous n’avez pas besoin de connexions sécurisées
Si vous n’avez pas besoin de SSL, vous pouvez supprimer ces DDL
- libcrypto-1_1-x64.dll
- libssl-1_1-x64.dll


Le répertoire d’exécution des utilisateurs Windows est le même emplacement que le .exe.

Pour les utilisateurs MAC, ce répertoire d’exécution est à 'PacketSender.app/Contents/MacOS'.
Si des fichiers INI sont trouvés, il les utilisera à la place de '%APPDATA%' ou 'Library/Application Support'.


<a id="smartresponses"></a>
## Réponses intelligentes

Packet Sender prend en charge jusqu’à 5 réponses intelligentes.

Pour activer cette fonctionnalité, accédez à _File -> Settings_ dans la barre d’outils de l’interface graphique. Accédez à l’onglet _Smart Responses_ et activez la case à cocher **Envoyer une réponse intelligente**.

![TCP direct de Packet Sender](screenshots/packetsender_smartreply.PNG)

* Packet Sender comparera le paquet dans l’encodage que vous choisissez.
* Packet Sender traduit l’encodage avant d’envoyer la réponse.
* Les encodages disponibles sont :
  * ASCII mixte - La méthode standard de codage de l’ASCII par l’expéditeur de paquets avec des caractères non imprimables
  * HEX -- Encodage HEX normal de Packet Sender


## Macros

Packet Sender prend en charge les macros suivantes lors de l’envoi de réponses :

* {{DATE}} -- Envoie la date actuelle au format « aaaa-mm-jj ».
* {{TIME}} -- Envoie l’heure actuelle au format « hh:mm:ss ap ».
* {{UNIXTIME}} -- Envoie l’horodatage de l’époque actuelle.
* {{RANDOM}} -- Envoie un nombre au hasard compris entre 0 et 32767 ou 2147483647, selon qu'il s'agit de 32 ou 64 bits (le programme d'installation par défaut pour Windows est de 32 bits. Mac est 64 bits).
* {{UNIQUE}} -- Envoie une chaîne aléatoire. Utilise un UUID interne pour le générer.
Packet Sender échangera la macro avec des valeurs réelles avant l’envoi.


<a id="persistent"></a>
## TCP et SSL persistants

Packet Sender prend en charge les connexions TCP et SSL persistantes via une fenêtre GUI distincte. Il est activé par une case à cocher dans la fenêtre principale ou via la fenêtre Paramètres.

![TCP direct de Packet Sender et SSL](screenshots/packetsender_direct_tcp.PNG)

### Remarques sur TCP et SSL persistants :
* N’importe quel nombre de connexions persistantes peut être créé.
* Les paquets précédemment enregistrés peuvent être chargés dans la liste déroulante.
* Il y a une vue « Brut » et une vue « ASCII ». La vue ASCII est utile pour dépanner les données qui ne sont pas imprimées par la vue brute.
* Le trafic est également enregistré dans le journal de trafic de la fenêtre principale.
* Un fichier peut être téléchargé sur la connexion permanente. Vous pouvez désactiver la journalisation si vous utilisez cette option.
* La minuterie en bas à gauche démarre dès qu’un paquet de données valide est envoyé/reçu. Il s’arrête lorsque la connexion est fermée.
* Vous pouvez éventuellement ajouter un retour chariot lorsque vous envoyez rapidement en appuyant sur la touche de retour. Ceci est utile pour les menus d’invite de commandes sur les connexions TCP / SSL. Packet Sender se souvient de l’état précédent de la case à cocher \r.
* Les connexions persistantes entrantes au serveur lanceront la fenêtre GUI séparée.
* Lors du renvoi, le paquet de connexion persistant est transféré dans la nouvelle fenêtre de l’interface graphique. Cliquez sur « Renvoyer(1) » pour l’annuler.

Les connexions permanentes ne sont pas prises en charge via la ligne de commande.


<a id="http"></a>
# HTTP / HTTPS POST & GET
Packet Sender prend en charge l’envoi de requêtes POST/GET via HTTP et HTTPS.
La liste déroulante Protocole inclut les options suivantes : HTTP GET, HTTP POST, HTTPS GET, HTTPS POST. Lors de la sélection de HTTP(S), les champs de saisie seront orientés vers : Nom, Demande, Adresse, Données (lorsque l’option POST est sélectionnée), bouton Générer des données (lorsque l’option POST est sélectionnée), Charger le fichier (lorsque l’option POST est sélectionnée).

## Envoi de requêtes HTTP/HTTPS GET/POST
![](/screenshots/ps_http_getfields.PNG)
* Sélectionnez HTTP(S) GET ou POST dans la liste déroulante du protocole
* Dans le champ *Adresse*, entrez le domaine ou l’adresse IP
* Dans le champ *Demande*, ajoutez le chemin de l’URL, si nécessaire
* Dans le champ *Port*, la valeur par défaut pour HTTP est 80 et HTTPS est 443
* Cochez *TCP persistant* pour voir plus clairement les données du serveur (les en-têtes HTTP sont supprimés automatiquement).

**Vous pouvez également coller une URL complète dans le champ Demande et Packet Sender analysera et remplira automatiquement les autres champs.**

### Pour les demandes POST :
* Vous pouvez ajouter manuellement les données dans le champ *Données*.
	* Le format irait: clé = valeur
	* Pour plusieurs paramètres : clé=valeur&clé=valeur&clé=valeur
* Ou vous pouvez cliquer sur le bouton * Générer des données *

<img src="/screenshots/ps_http_datagenerator.PNG" width="400" height="284">

* * Pour ajouter des données, saisissez les paramètres Clé et Valeur. Cliquez sur le bouton **+**.
* Peut ajouter plusieurs paramètres avec le bouton +.
* Supprimez les paramètres en cliquant sur le bouton X à côté du paramètre
* Une fois les paramètres ajoutés, cliquez sur OK et les données seront générées dans le champ Données.

### Pour ajouter des informations d’authentification :

<img src="/screenshots/ps_http_authgenerator.PNG" width="800" height="339">

* Allez dans Fichier -> Paramètres -> HTTP
* Cochez * Générer l’en-tête d’authentification *
* Entrez *Hôte*, *UN/Client ID* et *MP/Accès*
* Cliquez sur *HTTP Auth Header* pour générer l’en-tête d’authentification

<a id="panelgen"></a>
# Générateur de panneaux
Packet Sender prend en charge la génération de panneaux de contrôle. Les panneaux sont constitués de boutons auxquels sont affectés des scripts (paquets). Cliquez sur le bouton pour exécuter le(s) paquet(s) référencé(s) sur ce bouton.

<img src="/screenshots/ps_panel_1.PNG" width="400" height="358">

## Chargement d’un panneau
Les panneaux peuvent être créés de deux manières :
* Cliquez sur **Panneaux** dans la barre d’outils et sélectionnez Charger le panneau de démarrage ou Vider le projet de panneau
	* Le panneau de démarrage chargera le panneau assigné comme démarreur. Si aucun panneau n'est défini comme point de départ, un projet de panneau vide est alors ouvert.
* Mettez en surbrillance 2 paquets enregistrés ou plus et cliquez sur le bouton **Générer le panneau** (le bouton Générer le panneau n’apparaît que lorsque plusieurs paquets sont sélectionnés)

![](/screenshots/ps_panel_generate.PNG)


## Création de scripts pour un panneau
Pour commencer à écrire des scripts sur les boutons de votre panneau, vous devrez ouvrir un panneau et accéder à l’écran Édition. Une fois qu’un projet de panneau est ouvert, cochez le bouton dans le coin inférieur droit. Si ce bouton indique « Affichage », vous êtes sur l’écran Affichage. Cliquez sur le bouton pour déplacer le panneau vers l’écran Édition.

Une fois sur l’écran d’édition, des boutons et des scripts peuvent être ajoutés au panneau.

### Bouton de scripting
Les scripts de bouton contiendront le nom du paquet à envoyer.

<img src="/screenshots/ps_panel_2.PNG" width="400" height="360">

Plusieurs paquets peuvent être définis sur un bouton en ajoutant chaque nom sur une nouvelle ligne.

<img src="/screenshots/ps_panel_5.PNG" width="400" height="358">

Le Générateur de panneau supporte l'ajout d'un délai entre plusieurs paquets en ajoutant " délai:_# de secondes_ " entre les paquets.

<img src="/screenshots/ps_panel_4.PNG" width="400" height="358">

Le générateur de panneau prend en charge l'ajout d'un script pour charger un nouveau panneau en ajoutant " panneau:_ id du panneau #_ ". Une fois que tous les scripts précédents sur le bouton sont exécutés, le panneau passe au panneau suivant.

<img src="/screenshots/ps_panel_8.PNG" width="400" height="358">


### Ajout de fichiers/URL
Le générateur de panneau prend en charge l’ajout de boutons liés à des fichiers ou des URL stockés localement.
Les boutons Fichier/URL peuvent être ajoutés dans l’écran Édition en cliquant sur le bouton *+* dans le coin inférieur droit.
* Pour les fichiers : Accédez au fichier sur le PC, faites un clic droit sur le fichier et sélectionnez Copier. Collez-le dans la zone de texte _URL ou Ficher_ de Packet Sender
* Pour les URL : Copiez l’URL dans la zone de texte _URL ou Ficher_ de Packet Sender
	* Les URL doivent commencer par http:// ou https://


![](/screenshots/ps_panel_7.PNG)

Une fois le fichier ou l’URL copié, vous serez invité à entrer un nom pour le bouton. Les boutons seront remplis au bas du panneau.

Sur l’écran Édition, cliquer sur ces boutons vous permettra de modifier le lien fichier/URL et le nom du bouton. Vous pouvez également supprimer le bouton en cliquant sur **X** dans la fenêtre contextuelle.

![](/screenshots/ps_http_changeURL.PNG)

Lorsque vous êtes sur l’écran d’affichage, cliquez sur ces boutons pour lancer l’URL dans le navigateur par défaut ou ouvrir le fichier (avec le programme par défaut pour le type de fichier).


![](/screenshots/ps_panel_6.PNG)

### Édition/Enregistrement du panneau
Alors que dans l’écran d’édition d’un panneau, il y aura une barre d’outils avec les menus Fichier, Exporter, Paramètres, Aide. Vous pouvez enregistrer, exporter, importer, charger des projets de panneau et modifier le projet de panneau actuel à partir de cette barre d’outils.

Dans Paramètres, vous pouvez effectuer les opérations suivantes :
* Définir le nom du panneau - sélectionnez cette option pour renommer le projet de panneau actuel
* Définir l’ID du panneau - sélectionnez cette option pour modifier l’ID associé au projet de panneau en cours
_Note: La définition d’un ID en cours d’utilisation remplacera cette panneau_
* Panneau de démarrage - sélectionnez cette option pour définir le projet de panneau actuel comme panneau de démarrage.
* Supprimer le panneau - Cela fera apparaître un menu des projets de panneau en cours. Sélectionnez un projet de panneau pour le supprimer. _Note: Les boutons et les scripts seront conservés sur l’écran Modification jusqu’à ce que le panneau soit closed_



<a id="cli"></a>
# Ligne de commande
Packet Sender peut être utilisé à partir de la ligne de commande sur votre ordinateur.

Pour Windows, utilisez l’extension .com ('packetsender.com') pour utiliser l’interface de ligne de commande. En option, vous pouvez également utiliser 'packetsender' sans extension. En utilisant l’extension, .exe lancerez l’interface graphique.


![Capture d’écran de l’interface de ligne de commande de Packet Sender](screenshots/packetsender_command_line.png)

Pour Linux, le système de ligne de commande dans Packet Sender suit le même modèle que les autres utilitaires Linux. Il a un nom long (tel que --version) et un nom court (tel que -v). Ces options peuvent être organisées dans n’importe quel ordre et Packet Sender les analysera correctement. Les 3 dernières options sont positionnelles et doivent apparaître en dernier. Il s’agit de l’adresse IP, du port et des données. Ces dernières options sont facultatives si vous utilisez un paquet stocké.

```texte
packetsender --AIde
Packet Sender est un utilitaire de test réseau UDP / TCP / SSL / HTTP par NagleCode
Voir https://PacketSender.com/ pour plus d’informations.

Options:
  -?, -h, --help Affiche l’aide sur les options de ligne de commande.
  --help-all Affiche l’aide, y compris les options spécifiques à Qt.
  -v, --version Affiche les informations de version.
  -q, --mode silencieux. Seules les données de sortie ont été reçues.
  -x, --hex Analyser les données à envoyer en hexadécimal (par défaut pour
                            TCP/UDP/SSL).
  -a, --ascii Analyse les données à envoyer en ascii mixte (par défaut pour http
                            et GUI).
  -A, --ASCII Analyse les données à envoyer en ascii pur (pas de \xx).
                            traduction).
  -w, --wait <ms> Attendez jusqu’à <millisecondes> pour une réponse après
                            envoi. Zéro signifie ne pas attendre (par défaut).
  -f, --file <chemin> Envoie le contenu du chemin spécifié. Max 10 MiB pour
                            UDP, 100 Mio pour TCP/SSL.
  -b, --bind <port> port de liaison. La valeur par défaut est 0 (dynamique).
  -6, --ipv6                Force IPv6. Identique à -B « :: ». La valeur par défaut est IP:Any.
  -4, --ipv4                Force IPv4.  Identique à -B « 0.0.0.0 ». La valeur par défaut est
                            IP: N’importe lequel.
  -B, --bindip <IP> Lier l’adresse IP personnalisée. La valeur par défaut est IP:Any.
  -t, --tcp Send TCP (par défaut).
  -s, --ssl Envoyez SSL et ignorez les erreurs.
  -S, --SSL Envoyez SSL et arrêtez en cas d’erreur.
  -u, --udp Envoyer UDP.
  --http <http> Envoyer HTTP. Les valeurs autorisées sont GET (par défaut) et
                            POST
  -n, --name <nom> envoie le paquet précédemment enregistré nommé <nom>. Autre
                            Les options remplacent les paramètres de paquets enregistrés.
  --bps <bps> Trafic intense. Calculer le taux en fonction de la valeur de
                            bits par seconde.
  --num <nombre> Trafic intense. Nombre de paquets à envoyer. Faire défaut
                            illimité.
  --rate <Hertz> Trafic intense. Taux. Ignoré dans l’option bps.
  --usdelay <microsecondes> Trafic intense. Renvoyer le délai. Utilisé si le taux est égal à 0.
                            Ignoré dans l’option bps.

Arguments:
  adresse Adresse/URL de destination. Facultatif pour le paquet enregistré.
  port Port de destination/Données POST. Facultatif pour enregistré
                            paquet.
  données Données à envoyer. Facultatif pour le paquet enregistré.
```
## Exemple d’interface de ligne de commande
L’interface de ligne de commande suit le même format entre Windows, Linux et MAC.

Le format est : 'packetsender [options]adresse données port'

```texte
packetsender -taw 500 mirrors.xmission.com 21 « UTILISATEUR anonyme\r\nPASS chrome@example.com\r\n »
TCP (65505)://mirrors.xmission.com:21 55 53 45 52 20 61 6e 6f 6e 79 6d 6f 75 73 0d 0a 50 41 53 53 20 63 68 72 6f 6d 65 40 65 78 61 6d 70 6c 65 2e 63 6f 6d 0d 0a
Temps de réponse:5:51:37.042 pm
Réponse HEX:32 32 30 2D 57 65 6C 63 6F 6D 65 20...
Réponse ASCII:220-Bienvenue sur XMission Internet...
```

## Exemples de liaison à un port et à une adresse IP personnalisée, IPv4 ou IPv6

La ligne de commande de l’expéditeur de paquets peut se lier à des ports personnalisés pour forcer les modes IPv4/6 ou plusieurs cartes réseau à l’aide de l’option -B.
```texte
packetsender -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hello\r"
packetsender -taw 3000 192.168.0.201 5005 « Bonjour\r »
packetsender -B 192.168.0.200 -taw 3000 192.168.0.201 5005 « Bonjour\r »
packetsender -B fe80::a437:399a:3091:266a%ethernet_32769 -taw 3000 fe80::c07b:d517:e339:5a08 5005 « Bonjour\r »
packetsender -B fe80::a437:399a:3091:266a -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hello\r"
```

## Exemple d’interface de ligne de commande utilisant SSL et ignorant les erreurs

La ligne de commande a la possibilité d’ignorer ou d’abandonner les erreurs SSL. La valeur par défaut est d’ignorer.

* Utilisez l’option -s pour envoyer SSL et ignorer les erreurs.
* Utilisez l’option -S pour envoyer SSL et arrêter les erreurs

```texte
packetsender -saw 500 expired.packetsender.com 443 « GET / HTTP/1.0\r\n\r\n »
Erreur SSL : Le certificat a expiré
SSL (54202)://expired.packetsender.com:443 47 45 54 20 2f 20 48 54 54 50 2f 31 2e 30 0d 0a 0d 0a
Chiffrer: Chiffré avec AES(128)

Temps de réponse:3:24:55.695 pm
Réponse HEX:48 54 54 50 2f 31 2e 31 20 34 32 31 20 0d 0a 53 65 72 76 65 72 3a 20 6e 67 69 6e 78 2f 31 2e 31 30 2e 30 20 28 55 62 75 6e 74 75 29 0d
Réponse ASCII:HTTP/1.1 421 \r\nServeur: nginx/1.10.0 (Ubuntu)\r
```

## Exemple d’interface de ligne de commande utilisant HTTP
Notez que cela utilise les paquets par défaut intégrés.
```texte
packetsender --name « HTTPS POST Params »
packetsender --http GET « https://httpbin.org/get »
packetsender --http POST « https://httpbin.org/post » « {} »
```



<a id="cliintensetraffic"></a>

## Exemple d’utilisation de CLI Intense Traffic Generator
Le générateur de trafic intense de commande fonctionne à peu près de la même manière que la version GUI, mais il est un peu plus précis, avec plus d’options de contrôle (et plus d’intensité!).

Voir ci-dessous pour des exemples de son utilisation. Notez que ces calculs sont « Meilleur effort ». Il fonctionne bien, mais les pics de processeur ou divers hoquets réseau peuvent le décourager. Le filetage n’est pas en temps réel, et n’est pas super intelligent avec ses tentatives de compensation.

* Renvoyer « Mon paquet génial» à un taux de 20 Hz
* Renvoyez « Mon paquet génial » à un bps de 2000 bauds
* Renvoyez « Mon paquet génial » aussi vite que possible
* Renvoyez « Mon paquet génial » avec un délai de 2000000 microsecondes entre chaque paquet

**Note: Pour Windows, utilisez la version « .com », de sorte que chaque exemple soit packetsender.com**

```texte
packetsender --rate 20 --Nom « Mon paquet génial »
packetsender --bps 2000 --nom «Mon paquet génial »
packetsender --rate 0 --nom « Mon paquet génial »
packetsender --usdelay 2000000 --nom « Mon paquet génial »
```


<a id="building"></a>
# Création de packet sender
La seule dépendance est Qt SDK

## Création de packet sender pour Windows / MAC
1. Téléchargez le programme d’installation de Qt depuis http://www.qt.io/download-open-source/
1. Laissez-le installer MingGW si vous n’avez pas de compilateur.
1. Ouvrir le projet PacketSender.pro
1. créer!

Les versions Windows et Mac ont été créer en utilisant Qt 5.12. Packet Sender prend en charge Qt 6, mais il ne le fait pas
Soutenez CMAKE.

## Création de Packet Sender Pour Linux
Voici la séquence de commandes pour Ubuntu 16.04. Veuillez vous adapter à votre plate-forme Linux. Packet Sender ne nécessite aucune bibliothèque supplémentaire au-delà du SDK Qt stock. On m’a dit qu’il y avait des problèmes de construction avec Fedora stock. Si un assistant Fedora a des idées, faites-le moi savoir, et j’ajouterai vos instructions.

Si vous vous sentez aventureux, n’hésitez pas à construire à partir de la branche principale. Il contient la dernière version stable. La branche du développement devrait probablement être évitée.

```bash
sudo apt-get mise à jour
sudo apt-get install qt5-default build-essential
wget https://github.com/dannagle/PacketSender/archive/(Version).tar.gz
tar -xzvf (Version).tar.gz
cd PacketSender-(Version)/src
qmake PacketSender.pro
faire
```

Pour exécuter l’utilisation :
```texte
./PacketSender
```

S’il ne s’exécute pas, vous devrez peut-être le définir exécutable
```texte
chmod a+x PacketSender
```


# Améliorations/Demandes

Il vous manque une fonctionnalité ? Vous pouvez [m’engager pour l’ajouter à Packet Sender](https://packetsender.com/enhancements).

# Droit / Conformité

La licence est GPL v2 ou ultérieure. [Contactez-moi](https://packetsender.com/contact) si vous avez besoin d’une licence différente.
Certaines distributions de Packet Sender peuvent utiliser [OpenSSL](https://www.openssl.org/).
Le VPAT le plus récent [peut être trouvé](vpat_2.4_packetsender.pdf) dans ce référentiel.


# Droit d’auteur

Packet Sender a été écrit par [Dan Nagle](https://dannagle.com/) et est publié par &copy; NagleCode, LLC - [@NagleCode](https://twitter.com/NagleCode) - [PacketSender.com](https://packetsender.com)
