IF %1.==. GOTO No1
echo %1 > buildversion.txt
REM copy /Y C:\Users\danie\Desktop\prebuild-windows.php ci_cd\windows\win-prebuild.php
wsl php ci_cd/windows/win-prebuild.php

GOTO End1

:No1
  ECHO Need build version param (such as 7.2.1)
:End1
