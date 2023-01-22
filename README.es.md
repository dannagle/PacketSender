# Introducción

![Logotipo de Packet Sender](screenshots/packetsender_banner.png)

*Lea esto en otros idiomas: [English](README.md), [Español](README.es.md), [Deutsch](README.de.md).*


[![Descargas](https://img.shields.io/github/downloads/dannagle/PacketSender/total.svg)](https://packetsender.com/download)

Packet Sender es una utilidad de código abierto que permite enviar y recibir paquetes TCP, UDP y SSL (TCP encriptado), así como peticiones HTTP/HTTPS y generación de paneles. La rama mainline soporta oficialmente Windows, Mac y Linux de escritorio (con Qt). Otros lugares pueden recompilar y redistribuir Packet Sender. Packet Sender es gratuito y tiene licencia GPL v2 o posterior. Puede utilizarse tanto para uso comercial como personal. Si encuentra útil la aplicación, por favor considere donar/patrocinar para que el desarrollo pueda continuar.



# Índice
* [Patrocinadores](#sponsors)
  * Visite [IWL.com](https://www.iwl.com/)
  * Visite [NagleCode.com](https://dannagle.com/)

* [Interfaz de Usuario](#gui)
* [Generador de tráfico intenso (GUI)](#udptraffic)
* [Red](#network)
* [Descargas](#downloads)
* [Soporte](#support)
* [Calculadora de Subredes IPv4](#subnetcalculator)
* [Packet Sender en la Nube](#cloud)
* [Modo portátil](#portable)
* [Macros y respuestas inteligentes](#smartresponses)
* [TCP y SSL persistentes](#persistent)
* [Solicitudes HTTP/HTTPS](#http)
* [Generador de paneles](#panelgen)
* [Interfaz de línea de comandos](#cli)
* [Generador de tráfico intenso (CLI)](#cliintensetraffic)
* [Generador de paquetes](#building)




<a id="sponsors"></a>
## Patrocinadores

Packet Sender quiere dar las gracias a los siguientes patrocinadores.

[![IWL](screenshots/iwl_logo.png)](https://www.iwl.com/)
<br>IWL es una empresa californiana que crea productos para redes informáticas.
<br><br><br>

[![NagleCode](screenshots/naglecode-logo400.png)](https://dannagle.com)
<br>NagleCode es un editor de software y estudio de desarrollo. 
<br><br><br>


[¿Desea que su nombre/logo aparezca aquí?](https://github.com/sponsors/dannagle)



<a id="support"></a>
## Soporte

* Twitter: [@NagleCode](http://twitter.com/NagleCode)
* Los foros están en: [Discusiones en GitHub](https://github.com/dannagle/PacketSender/discussions).
* Correo electrónico: [Contacto de Packet Sender](https://packetsender.com/contact)
* Contácteme en [LinkedIn](https://www.linkedin.com/in/dannagle/)

*NOTA:* Pruebe a desactivar (temporalmente) su cortafuegos si tiene problemas en Windows.

<a id="downloads"></a>
# Descargas

## Descarga de Escritorio
Las versiones oficiales de Packet Sender pueden descargarse en [PacketSender.com](http://packetsender.com/download). Algunos sitios redistribuyen Packet Sender.

![Logo de Windows](screenshots/winlogo150.png) ![Logo de Mac](screenshots/maclogo150.png) ![Logo de Linux](screenshots/Tux150.png)


<!--
No estoy seguro de cuándo volverá esto.

## Aplicaciones móviles
Las ediciones móviles de Packet Sender son totalmente nativas, contienen los permisos mínimos y no recopilan datos. Este es un software que le respeta. Gracias por apoyar este esfuerzo.


### Aplicación Móvil para Android
![Logotipo de Android](screenshots/android_logo.png)

La versión para Android se encuentra [en Google Play](https://play.google.com/store/apps/details?id=com.packetsender.compose) o [en Amazon Appstore](https://www.amazon.com/dp/B08RXM6KM2/)

[![Packet Sender para Android](screenshots/packetsender_android_screenshot.png)](https://play.google.com/store/apps/details?id=com.packetsender.compose

-->


<!--
No estoy seguro de cuándo estará de vuelta.

## Aplicación móvil para iOS

Logotipo de Packet Sender](screenshots/ios_logo.png)

La versión iOS se encuentra [en la Apple App Store](https://apps.apple.com/app/id1558236648#?platform=iphone)

[![Packet Sender para iOS](screenshots/packetsender-ios-traffic-log-ascii.png)](https://apps.apple.com/app/id1558236648#?platform=iphone)

-->
<a id="gui"></a>
# INTERFAZ DE USUARIO 

Packet Sender es idéntico para todas las versiones de escritorio. La única diferencia es su diseño para que coincida con el sistema operativo.

![Captura de pantalla de Packet Sender](screenshots/ps_GUI.png)

1. Un paquete tiene asociado un nombre, una dirección de destino (los nombres de dominio se resuelven por defecto justo antes del envío), un puerto y datos.
2. En la tabla, hay una lista de los paquetes guardados. Puede hacer doble clic para editar directamente los campos de esta tabla.
3. En la parte inferior derecha, aparecen los estados y puertos de los servidores UDP, TCP y SSL. Haciendo clic sobre ellos activará o desactivará el protocolo. Packet Sender admite la vinculación a cualquier número de puertos.
4. También hay un botón de alternancia IP. Al hacer clic en él se cambia a IPv4 (por defecto), IPv6 o IP personalizada.

### Notas generales
* Un valor de reenvío de "0" significa que es un paquete de un solo envío.
* Durante el reenvío de paquetes, habrá un botón para cancelar todos los reenvíos.
* Por favor, compruebe su cortafuegos. Windows bloquea agresivamente los servidores basados en TCP. Packet Sender seguirá funcionando si el cortafuegos lo bloquea, pero no podrá recibir paquetes no solicitados basados en TCP.
* Se puede enviar una respuesta opcional. Se utiliza la misma respuesta para TCP, UDP y SSL.
* Para el envío IPv6, también necesitará el ID de ámbito.
* Packet Sender admite notación mixta ASCII y HEX:
  * Haga doble clic en cualquiera de los campos para que aparezca el editor multilínea.
  * \XX se traduce a XX en hexadecimal
  * \n, \r, \t se traducirán a 0A, 0D y 09
  * Los números HEX están delimitados por espacios
    * El campo HEX intentará interpretar otros delimitadores comunes (como comas, dos puntos (Wireshark), punto y coma, " 0x", etc) y autocorregirse. Es muy tolerante a fallos.
    * También admite un único flujo HEX. Si el número de bytes es impar, Packet Sender asumirá que el byte frontal necesita un cero y entonces auto-corregirá.
  * Ejemplo ASCII: hola mundo\r
  * Ejemplo HEX: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0d
  * Puede guardar un paquete directamente desde el registro de tráfico. Se le pedirá un nombre, y la dirección de origen y el puerto se cambiarán para su comodidad.
  * También puede cargar un archivo directamente en el campo HEX. El campo HEX admite el envío de hasta 10.922 bytes. El límite teórico para el envío a través de la línea de comandos es de 200 MB.


## Teclas de acceso rápido/accesos directos del teclado

Puede navegar por los campos de la parte superior utilizando CTRL+1, CTRL+2, etc, hasta CTRL+8 (botón de envío). En Mac, la tecla de acceso directo es Comando.

Las teclas de acceso directo y los campos son:
* CTRL + 1 = Nombre
* CTRL + 2 = ASCII
* CTRL + 3 = HEXADECIMAL
* CTRL + 4 = Dirección
* CTRL + 5 = Puerto
* CTRL + 6 = Retardo de reenvío
* CTRL + 7 = Selección de protocolo
* CTRL + 8 = Enviar (ejecuta)


Algunas notas:
* Los campos están vinculados a la tecla de acceso directo correspondiente independientemente de la selección de protocolo.
* Cuando navegue a la opción TCP/UDP/SSL, puede utilizar los caracteres arriba/abajo o t/u/s/h para realizar una selección.
* Si va a automatizar con teclas de acceso rápido (utilizando herramientas como [AutoHotKey](https://www.autohotkey.com/)), puede que desee desactivar "Restaurar sesión anterior".



<a id="network"></a>
# Funciones de red

## IPv4, IPv6, IP personalizada

Los servidores incorporados de Packet Sender están configurados para soportar IPv4 o IPv6 pero no ambos al mismo tiempo. Para los clientes, la GUI y CLI de Packet Sender cambiarán sin problemas entre los dos modos en el momento del envío (el scope ID puede ser necesario para IPv6). Pulse el switch IPv4 / IPv6 en la parte inferior derecha para cambiar entre los dos.

Dentro de la configuración, también puede forzar a los servidores de Packet Sender a enlazarse a una dirección IP personalizada. Esto puede ser muy útil para sistemas con múltiples NICs o configuraciones IP complicadas. Packet Sender provocará un error si se le indica que se vincule a una dirección que no existe.

![Enlace específico de IP](screenshots/ip-specific-binding.png)


<a id="subnetcalculator"></a>
## Calculadora de subredes IPv4

Packet Sender tiene una calculadora de subredes incorporada. Se encuentra en el menú Herramientas.
![Calculadora de Subredes de Packet Sender](screenshots/packetsender_subnetcalc.PNG)

* La ventana de registro (sección inferior) mostrará las direcciones IPv4 e IPv6 sin loopback encontradas en su ordenador.
* En el lado izquierdo, introduzca la dirección IPv4 en el campo IP. 
* En el lado izquierdo, introduzca la subred en notación X.X.X. o /XX.
* Los resultados del cálculo aparecen a la derecha.
* El campo de abajo es una comprobación rápida para ver si una IPv4 está dentro de una de sus subredes.

## Cliente y servidor SSL

Packet Sender soporta el establecimiento de conexiones encriptadas sobre SSL.
Esto es soportado en la GUI y en la línea de comandos.

Packet Sender incluye OpenSSL para su uso en Windows. En Mac y Linux, Packet Sender utilizará las librerías SSL nativas.

_Este producto incluye software desarrollado por el Proyecto OpenSSL para su uso en el OpenSSL Toolkit. (http://www.openssl.org/)_

![Direct TCL de Packet Sender](screenshots/packetsender_ssl.PNG)

Notas sobre SSL:
* La negociación del certificado se gestiona inmediatamente durante la conexión.
* Por defecto, Packet Sender ignora todos los errores SSL (cert caducado, nombre de host incorrecto, autofirmado, etc).
* Packet Sender muestra el progreso de la negociación del certificado en el registro de tráfico.
* Packet Sender emite el algoritmo de encriptación utilizado en el registro de tráfico (como AES 128).

Packet Sender incluye un certificado interno "Snake Oil" para su uso como servidor para Windows. El certificado y la clave se encuentran en el mismo lugar que el paquete y la configuración.

_Nota: Anular las ubicaciones del certitifcado en Ajustes también anula el certificado Snake-Oil._

Si se produce un error SSL, Packet Sender lo mostrará en el registro de tráfico. Si la configuración es continuar de todas formas (por defecto), continuará negociando la encriptación. En caso contrario, la conexión finalizará con un fallo de conexión.

![Packet Sender Direct TCP Expired](screenshots/packetsender_expired_ssl.png)

## Multidifusión (Experimental)

El soporte multidifusión de Packet Sender se activa al intentar enviar a una dirección multidifusión IPv4 o desde el submenú multidifusión. La característica es actualmente experimental y tiene estos problemas conocidos.

* Packet Sender abandona el soporte IPv6 cuando se une a una multidifusión.
* Y permanece abandonado hasta que se revisa la configuración o intenta rrealizar un envío a IPv6
* En wifi, a veces tarda 20 segundos para que la unión a multidifusión surta efecto.
* Paclket Sender no tiene lógica para volver a unirse a un grupo multidifusión si el router se reinicia o se produce algún otro error común.

No hay soporte para multidifusión IPv6, aunque está en la hoja de ruta. Los patrocinadores que deseen soporte para multidifusión IPv6 pueden contactarme.

<a id="udptraffic"></a>

## Generador de tráfico UDP (Experimental)

Para cuando el sistema de envío normal no es suficiente, puede martillear una IP objetivo con paquetes para ver si su dispositivo puede manejarlo. Puede encontrarlo en la barra de herramientas de la GUI en _Herramientas -> Generador de tráfico intenso_.

Tenga en cuenta que esta función es experimental y que las métricas mostradas no se han probado completamente. Para una prueba más precisa, puede que desee consultar la versión CLI de esta herramienta.

![Enlace específico IP](screenshots/udp-traffic-sending.PNG)

# Características del Packet Sender


<a id="cloud"></a>
## Nube del Packet Sender

Los conjuntos de paquetes pueden guardarse/recuperarse/compartirse rápidamente utilizando el servicio gratuito [Packet Sender Cloud](https://cloud.packetsender.com/). La nube también puede utilizarse para mostrar y distribuir públicamente sus paquetes (a través de una URL) para colaboración, tutoriales, usuarios finales, etc. Packet Sender puede importar conjuntos de paquetes públicos con una URL pública.

Hay varias razones para hacer esto:

* Mantener todos sus paquetes listos para poder recuperarlos rápidamente al instalar un Packet Sender nuevo.
* Intercambiar rápidamente entre conjuntos de paquetes cuando trabaje en diferentes proyectos.
* Compartir un inicio de sesión (está permitido) para la generación colaborativa de conjuntos de paquetes.
* Tener una página pública de sus conjuntos de paquetes para que otros puedan encontrarlos e importarlos rápidamente

![Importación en la nube de Packet Sender](screenshots/cloud-import.png)

Si está publicando una API de red, mantener una página pública de la nube es significativamente más fácil que detallar penosamente (IP, puerto, tipo, etc.) los paquetes a sus usuarios. Además, actualizar esa página es fácil.

Encontrará más información al respecto en
https://cloud.packetsender.com/help


<a id="portable"></a>
## Modo portátil

Packet Sender tiene un modo "portable". Al iniciarse, buscará `portablemode.txt` y rellenará los archivos de configuración que falten en ese directorio de ejecución. Estos archivos son `packets.ini`, `ps_settings.ini`, `ps.key`, y `ps.pem`.
También puede tener algunos archivos portables y los demás en su ubicación estándar eliminando portablemode.txt.

### DDLs que pueden eliminarse en Modo portable sólo por consola
Si no necesita la GUI, puede eliminar estos DDLs
- Qt5Svg.dll
- libEGL.dll
- libGLESv2.dll
- Qt5Widgets.dll
- Qt5Gui.dll
- opengl32sw.dll
- D3Dcompiler_47.dll
- directorio iconengines
- directorio imageformats
- directorio styles

### DDLs que pueden eliminarse si no necesita conexiones seguras
Si no necesita SSL, puede eliminar estas DDL
- libcrypto-1_1-x64.dll
- libssl-1_1-x64.dll


El directorio de tiempo de ejecución usuarios de Windows es el mismo lugar que el .exe.

Para los usuarios de MAC, este directorio de tiempo de ejecución está en `PacketSender.app/Contents/MacOS`.
Si se encuentran archivos INI, los utilizará en lugar de `%APPDATA%` o `Library/Application Support`.


<a id="smartresponses"></a>
## Respuestas inteligentes

Packet Sender soporta hasta 5 respuestas inteligentes. 

Para activar esta función, vaya a _Archivo -> Configuración_ en la barra de herramientas de la GUI. Vaya a la pestaña _Respuestas inteligentes_ y active la casilla **Enviar una respuesta inteligente**.  

![Packet Sender - TCP Directo](screenshots/packetsender_smartreply.PNG)

* Packet Sender comparará el paquete dentro de la codificación que usted elija.
* Packet Sender traduce la codificación antes de enviar la respuesta.
* Las codificaciones disponibles son:
  * ASCII mixto -- La forma estándar de Packet Sender de codificar ASCII junto con caracteres no imprimibles.
  * HEX -- La codificación HEX normal de Packet Sender


## Macros

Packet Sender soporta estas macros cuando envía respuestas:

* {{DATE}} -- Envía la fecha actual en formato "aaa-mm-dd".
* {{TIME}} -- ENVÍA LA HORA ACTUAL EN FORMATO "AAAA-MM-DD". -- Envía la hora actual en formato "hh:mm:ss ap".
* {{UNIXTIME}} -- Envía la marca de tiempo del momento actual.
* {{RANDOM}} -- Envía un número aleatorio que oscila entre 0 y 32767 o 2147483647, según sea de 32 o 64 bits (el instalador por defecto para Windows es de 32 bits. Mac es de 64 bits).
* {{UNIQUE}} -- Envía una cadena aleatoria. Utiliza un UUID interno para generarla.
El Packet Sender intercambiará la macro con valores reales antes de enviarla.


<a id="persistent"></a>
## TCP y SSL persistentes

Packet Sender soporta conexiones TCP y SSL persistentes a través de una ventana GUI separada. Se activa mediante una casilla de verificación en la ventana principal o a través de la ventana de Configuración.

![Packet Sender TCP directo y SSL](screenshots/packetsender_direct_tcp.PNG)

### Notas sobre TCP y SSL persistentes:
* Se puede crear cualquier número de conexiones persistentes.
* Se pueden cargar paquetes previamente guardados en el desplegable.
* Hay una vista "Raw" y una vista "ASCII". La vista ASCII es útil para solucionar problemas con los datos que no imprime la vista en Raw.
* El tráfico también se guarda en el registro de tráfico de la ventana principal.
* Se puede cargar un archivo en la conexión persistente. Es posible que desee desactivar el registro si utiliza esta opción.
* El temporizador de la parte inferior izquierda se inicia en cuanto se envía/recibe un paquete de datos válido. Se detiene cuando se cierra la conexión.
* Puede añadir opcionalmente un retorno de carro cuando realice un envío rápido pulsando la tecla de retorno. Esto es útil para los menús de solicitud de comandos a través de conexiones TCP / SSL. El Packet Sender recuerda el estado anterior de la casilla \r.
* Las conexiones persistentes entrantes al servidor lanzarán la ventana GUI separada.
* Durante el reenvío, el paquete de conexión persistente se traslada a la nueva ventana GUI. Al hacer clic en "Reenviar(1)" se cancelará.

Las conexiones persistentes no son compatibles a través de la línea de comandos.


<a id="http"></a>
# HTTP/HTTPS POST y GET
Packet Sender admite el envío de solicitudes POST/GET a través de HTTP y HTTPS. 
El desplegable de protocolo incluye las siguientes opciones: HTTP GET, HTTP POST, HTTPS GET, HTTPS POST. Al seleccionar HTTP(S), los campos de entrada udpatearán a: Nombre, Solicitud, Dirección, Datos (cuando se selecciona POST), Botón Generar Datos (cuando se selecciona POST), Cargar Fichero (cuando se selecciona POST). 

## Envío de solicitudes HTTP/HTTPS GET/POST
![](/screenshots/ps_http_getfields.PNG)
* Seleccione HTTP(S) GET o POST en el desplegable de protocolo
* En el campo *Address* introduzca el dominio o la IP
* En el campo *Request* añada la ruta URL, si es necesario
* En el campo *Port*, el valor por defecto para HTTP es 80 y HTTPS es 443
* Marque *Persistent TCP* para ver los datos del servidor con mayor claridad (las cabeceras HTTP se eliminan automáticamente). 

**También puede pegar una URL completa en el campo Solicitud y Packet Sender analizará y rellenará automáticamente los demás campos.**

### Para solicitudes POST:
* Puede añadir manualmente los datos en el campo *Data*.
	* El formato sería: clave=valor
	* Para múltiples parametros: clave=valor&clave=valor&clave=valor
* O puede hacer clic en el botón *Generar datos

<img src="/screenshots/ps_http_datagenerator.PNG" width="400" height="284">

* Para añadir datos, introduzca los parámetros Clave y Valor. Haga clic en el botón **+**. 
* Puede añadir múltiples parámetros con el botón +. 
* Elimine parámetros haciendo clic en el botón X situado junto al parámetro.
* Una vez añadidos los parámetros, pulse Ok y los datos se generarán en el campo Datos. 

### Para añadir credenciales de autenticación:

<img src="/screenshots/ps_http_authgenerator.PNG" width="800" height="339">

* Vaya a Archivo -> Configuración -> HTTP
* Marque *Generar cabecera de autenticación*
*Introduzca el *Host*, *UN/ID de Cliente*, y *PW/Acceso*.
* Pulse en *Cabecera de Autenticación HTTP* para generar la cabecera de autenticación

<a id="panelgen"></a>
# Generador de paneles
Packet Sender soporta la generación de paneles de control. Los paneles consisten en botones con scripts (paquetes) asignados a ellos. Al hacer clic en el botón se ejecutará el paquete o paquetes referenciados en ese botón. 

<img src="/screenshots/ps_panel_1.PNG" width="400" height="358">

## Cargar un panel
Los paneles pueden crearse de dos maneras:
* Haciendo clic en **Paneles** en la barra de herramientas y seleccionando Cargar Panel de Inicio o Proyecto de Panel Vacío
	* Cargar panel de inicio cargará el panel asignado como inicio. Si no hay ningún panel asignado como iniciador, se abrirá un proyecto de panel vacío. 
* Resaltar 2 o más paquetes guardados y hacer clic en el botón **Generar Panel** (el botón Generar Panel sólo aparece cuando se seleccionan varios paquetes) 

![](/screenshots/ps_panel_generate.PNG)


## Creando un Script para un Panel
Para empezar a scriptear los botones de su panel, necesitará abrir un panel e ir a la pantalla de Edición. Una vez abierto el proyecto de un panel, compruebe el botón de la esquina inferior derecha. Si este botón dice "Visualización", se encuentra en la pantalla Visualización. Pulse el botón para pasar el panel a la pantalla de Edición. 

Una vez en la pantalla de Edición, se pueden añadir botones y scripts al panel. 

### Scripts de botones
Los scripts de los botones contendrán el nombre del paquete a enviar. 

<img src="/screenshots/ps_panel_2.PNG" width="400" height="360">

Se pueden asignar varios paquetes a un botón añadiendo cada nombre en una nueva línea.

<img src="/screenshots/ps_panel_5.PNG" width="400" height="358">

El generador de paneles permite añadir un retardo entre varios paquetes añadiendo "retardo:_# de segundos_" entre paquete y paquete.

<img src="/screenshots/ps_panel_4.PNG" width="400" height="359">

El generador de paneles admite la adición de un script para cargar un nuevo panel añadiendo "panel:_panel id #_". Una vez ejecutados todos los scripts anteriores del botón, el Panel pasará al siguiente panel. 

<img src="/screenshots/ps_panel_8.PNG" width="400" height="358">


### Añadir archivos/URLs
El Generador de paneles admite la adición de botones que enlazan con archivos o URL almacenados localmente. 
Los botones de archivo/URL pueden añadirse mientras se está en la pantalla de edición haciendo clic en el *+* de la esquina inferior derecha. 
* Para archivos: Vaya al archivo en el PC, haga clic con el botón derecho en el archivo y seleccione Copiar. Péguelo en el cuadro de texto _URL o Archivo_ en Packet Sender
* Para URLs: Copie la URL en el cuadro de texto _URL o Archivo_ en Packet Sender
	* Las URL deben empezar por http:// o https://


![](/screenshots/ps_panel_7.PNG) 

Una vez copiado el archivo o la URL, se le pedirá que introduzca un nombre para el botón. Los botones aparecerán en la parte inferior del panel. 

Mientras se encuentre en la pantalla de edición, al hacer clic en estos botones podrá editar el enlace del archivo/URL y el nombre del botón. También puede eliminar el botón haciendo clic en la **X** de la ventana emergente. 

![](/screenshots/ps_http_changeURL.PNG)

En la pantalla de visualización, al hacer clic en estos botones se iniciará la URL en el navegador predeterminado o se abrirá el archivo (con el programa predeterminado para el tipo de archivo). 


![](/screenshots/ps_panel_6.PNG) 

### Edición/Guardado de Paneles
Mientras esté en la pantalla de edición de un Panel, habrá una barra de herramientas con los menús Archivo, Exportar, Configuración, Ayuda. Desde esta barra de herramientas puede guardar, exportar, importar, cargar proyectos de paneles y editar el proyecto del panel actual. 

Desde Ajustes, puede hacer lo siguiente:
* Establecer nombre de panel - seleccione esto para renombrar el proyecto de panel actual.
* Establecer ID del panel - seleccione esto para cambiar el ID asociado con el proyecto de panel actual
_Nota: Ajustar un ID en uso reemplazará ese panel_.
* Panel de inicio - seleccione esto para establecer el proyecto de panel actual como panel de inicio. 
* Borrar Panel - Esto mostrará un menú de los proyectos de panel actuales. Seleccione un proyecto de panel para eliminarlo. _Nota: Los botones y scripts se mantendrán en la pantalla de edición hasta que se cierre el panel_.



<a id="cli"></a>
# Línea de comandos
Packet Sender puede utilizarse desde la línea de comandos de su ordenador. 

Para Windows, utilice la extensión .com (`packetsender.com`) para utilizar la interfaz de la línea de comandos. Opcionalmente, también puede utilizar `packetsender` sin extensión. Si utiliza la extensión .exe iniciará la interfaz gráfica de usuario. 


![Captura de pantalla de CLI de Packet Sender](screenshots/packetsender_command_line.png)

Para Linux, el sistema de línea de comandos de Packet Sender sigue el mismo patrón que otras utilidades de Linux. Tiene un nombre largo (como --version) y un nombre corto (como -v). Estas opciones pueden disponerse en cualquier orden y Packet Sender las analizará correctamente. Las 3 últimas opciones son posicionales y deben aparecer en último lugar. Son IP, puerto y datos. Estas últimas opciones son opcionales si se utiliza un paquete almacenado.

```text
packetsender --help
Packet Sender es una utilidad de prueba de red UDP/TCP/SSL/HTTP de NagleCode.
Consulte https://PacketSender.com/ para más información.

Opciones:
  -?, -h, --help            Muestra ayuda sobre las opciones de la línea de comandos.
  --help-all                Muestra la ayuda incluyendo opciones específicas de Qt.
  -v, --version             Muestra información sobre la versión.
  -q, --quiet               Modo silencioso. Sólo muestra los datos recibidos.
  -x, --hex                 Analiza los datos a enviar como hexadecimales (por defecto para
                            TCP/UDP/SSL).
  -a, --ascii               Analiza los datos a enviar en formato mixto-ascii (por defecto para http
                            y GUI).
  -A, --ASCII               Analiza los datos a enviar como ascii puro (sin
                            traducción \xx).
  -w, --wait <ms>           Se esperará hasta <millisegundos> para una respuesta después del
                            envío. Un valor de cero significa sn espera (Por defecto).
  -f, --file <path>         Enviar el contenido de la ruta especificada. Máximo 10 MiB para
                            UDP, 100 MiB para TCP/SSL.
  -b, --bind <port>         Vincular puerto. Por defecto es 0 (dinámico).
  -6, --ipv6                Forzar IPv6. Igual que -B "::". Por defecto es IP:Any.
  -4, --ipv4                Forzar IPv4.  Igual que -B "0.0.0.0". Por defecto es IP:Any.                  
  -B, --bindip <IP>         Enlazar IP personalizada. Por defecto es IP:Any.
  -t, --tcp                 Enviar TCP (por defecto).
  -s, --ssl                 Enviar SSL e ignorar errores.
  -S, --SSL                 Enviar SSL y parar por errores.
  -u, --udp                 Enviar UDP.
  --http <http>             Enviar HTTP. Los valores permitidos son GET (por defecto) y
                            POST
  -n, --name <name>         Enviar el paquete previamente guardado llamado <name>. Otras
                            opciones anulan los parámetros del paquete guardado.
  --bps <bps>               Tráfico intenso. Calcula la tasa basándose en el valor de
                            bits por segundo.
  --num <number>            Tráfico intenso. Número de paquetes a enviar. Por defecto
                            ilimitado.
  --rate <Hertz>            Tráfico intenso. Tasa. Se ignora en la opción bps.
  --usdelay <microseconds>  Tráfico intenso. Retardo de reenvío. Utilizado si la tasa es 0.
                            Ignorado en la opción bps.

Argumentos:
  address                   Dirección/URL de destino. Opcional para paquete guardado.
  port                      Puerto de destino/datos POST. Opcional para el paquete
                            guardado
  data                      Datos a enviar. Opcional para el paquete guardado.
```
## Ejemplo de CLI
El CLI sigue el mismo formato entre Windows, Linux y MAC. 

El formato es `packetsender [options] puerto de dirección de datos`

```text
packetsender -taw 500 mirrors.xmission.com 21 "USER anonimo\nPASS chrome@example.com\r\n"
TCP (65505)://mirrors.xmission.com:21 55 53 45 52 20 61 6e 6f 6e 79 6d 6f 75 73 0d 0a 50 41 53 53 20 63 68 72 6f 6d 65 40 65 78 61 6d 70 6c 65 2e 63 6f 6d 0d 0a
Tiempo de respuesta:5:51:37.042 pm
Respuesta HEX:32 32 30 2D 57 65 6C 63 6F 6D 65 20...
Respuesta ASCII:220-Bienvenido a XMission Internet...
```

## Ejemplos de vinculación a puerto e IP personalizada, IPv4 o IPv6

La línea de comandos de Packet Sender puede vincularse a puertos personalizados para forzar modos IPv4/6 o múltiples NIC utilizando la opción -B.
```text
packetsender -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hola\r"
packetsender -taw 3000 192.168.0.201 5005 "Hello\r"
packetsender -B 192.168.0.200 -taw 3000 192.168.0.201 5005 "Hola\r"
packetsender -B fe80::a437:399a:3091:266a%ethernet_32769 -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hola\r"
packetsender -B fe80::a437:399a:3091:266a -taw 3000 fe80::c07b:d517:e339:5a08 5005 "Hola\r"
```

## Ejemplo de CLI utilizando SSL e ignorando errores

La línea de comandos tiene la opción de ignorar o abandonar en los errores SSL. El valor por defecto es ignorar.

* Utilice la opción -s para enviar SSL e ignorar errores.
* Utilice la opción -S para enviar SSL y abandonar en caso de errores

```text
packetsender -saw 500 expired.packetsender.com 443 "GET / HTTP/1.0\r\n\r\n"
Error SSL: El certificado ha caducado
SSL (54202)://expired.packetsender.com:443 47 45 54 20 2f 20 48 54 54 50 2f 31 2e 30 0d 0a 0d 0a
Cifrado: Cifrado con AES(128)

Tiempo de respuesta:3:24:55.695 pm
Respuesta HEX:48 54 54 50 2f 31 2e 31 20 34 32 31 20 0d 0a 53 65 72 76 65 72 3a 20 6e 67 69 6e 78 2f 31 2e 31 30 2e 30 20 28 55 62 75 6e 74 75 29 0d
Respuesta ASCII:HTTP/1.1 421 \r\nServidor: nginx/1.10.0 (Ubuntu)\r
```

## Ejemplo CLI usando HTTP
Tenga en cuenta que esto utiliza los paquetes incorporados por defecto.
```text
packetsender --name "Parámetros POST HTTPS"
packetsender --http GET "https://httpbin.org/get"
packetsender --http POST "https://httpbin.org/post" "{}"
```



<a id="cliintensetraffic"></a>

## Ejemplo utilizando el Generador de Tráfico Intenso CLI
El generador de tráfico intenso por comandos funciona de forma muy parecida a la versión GUI, pero es un poco más preciso, con más opciones de control, (¡y más intensidad!).

Vea a continuación ejemplos de cómo utilizarlo. Tenga en cuenta que estos cálculos son "Best Effort". Lo hace bien, pero los picos del procesador o diversos contratiempos de la red pueden echarlo a perder. El threading  no es en tiempo real, y no es superinteligente con sus intentos de compensar.

* Reenviar "Mi Impresionante Paquete" a una velocidad de 20 Hz
* Reenviar "Mi Impresionante Paquete" a una velocidad de 2000 baudios
* Reenviar "Mi Impresionante Paquete" lo más rápido posible
* Reenviar "Mi Impresionante Paquete" con un retardo de 2000000 microsegundos entre cada paquete

**Nota: Para Windows, use la construcción ".com", así que cada ejemplo sería packetsender.com**

```text
packetsender --rate 20 --name "Mi impresionante paquete"
packetsender --bps 2000 --name "Mi impresionante paquete"
packetsender --rate 0 --name "Mi impresionante paquete"
packetsender --usdelay 2000000 --name "Mi impresionante paquete"
```


<a id="building"></a>
# Activando Packet Sender
La única dependencia es Qt SDK

## Activando Packet Sender para Windows/MAC
1. Descargue el instalador de Qt desde http://www.qt.io/download-open-source/
1. Deje que instale MingGW si no tiene un compilador.
1. Abra el proyecto PacketSender.pro
1. Active. 

Las versiones para Windows y Mac fueron creadas usando Qt 5.12. Packet Sender soporta Qt 6, sin embargo no soporta cmake.

## Activando Packet Sender para Linux
Esta es la secuencia de comandos para Ubuntu 16.04. Por favor, adáptela a su plataforma Linux. Packet Sender no requiere librerías adicionales más allá del Qt SDK de stock. Me han dicho que hay problemas de compilación con Fedora stock. Si un asistente de Fedora tiene alguna idea, por favor hágamelo saber y añadiré sus instrucciones.

Si se siente aventurero, siéntase libre de realizar la instalación de la Ramificación Maestra. Contiene la última compilación estable. Probablemente debería evitar la rama de desarrollo.

```bash
sudo apt-get update
sudo apt-get install qt5-default build-essential
wget https://github.com/dannagle/PacketSender/archive/(Versión).tar.gz
tar -xzvf (Versión).tar.gz
cd PacketSender-(Versión)/src
qmake PacketSender.pro
make
```

Para ejecutarlo utilice
```text
./PacketSender
```

Si no se ejecuta, puede que tenga que configurarlo como ejecutable
```text
chmod a+x PacketSender
```


# Mejoras/Solicitudes

¿Echa en falta alguna característica? Puede [contratarme para que la añada a Packet Sender](https://packetsender.com/enhancements).

# Legal / Cumplimiento

La licencia es GPL v2 o posterior. [Póngase en contacto conmigo](https://packetsender.com/contact) si necesita una licencia diferente.
Algunas distribuciones de Packet Sender pueden utilizar [OpenSSL](https://www.openssl.org/).
El VPAT más actual [puede encontrarse](vpat_2.4_packetsender.pdf) en este repo.


# Derechos de autor

Packet Sender fue escrito por [Dan Nagle](https://dannagle.com/) y es publicado por &copy; NagleCode, LLC - [@NagleCode](https://twitter.com/NagleCode) - [PacketSender.com](https://packetsender.com)

