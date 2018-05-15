set BUILD_VERSION=5.7.%BUILD_NUMBER%
echo %BUILD_VERSION% > buildversion.txt
REM copy /Y C:\Users\Dan\Desktop\prebuild-windows.php ci_cd\windows\win-prebuild.php
C:\cygwin64\bin\php.exe ci_cd/windows/win-prebuild.php
