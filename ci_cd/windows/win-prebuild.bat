set BUILD_VERSION=1
set BUILD_VERSION=6.2.%BUILD_NUMBER%
echo %BUILD_VERSION% > buildversion.txt
REM copy /Y C:\Users\danie\Desktop\prebuild-windows.php ci_cd\windows\win-prebuild.php
wsl php ci_cd/windows/win-prebuild.php
