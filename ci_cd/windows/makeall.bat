IF %1.==. GOTO No1
PUSHD %TMP%
rd /s /q PacketSender
git clone --depth 1 -b development git@github.com:dannagle/PacketSender.git
cd PacketSender
REM Setting Version Numbers
call ci_cd\windows\win-prebuild.bat %1
REM Building app
call ci_cd\windows\win-build.bat
REM Building installers
call ci_cd\windows\win-deploy.bat
POPD

GOTO End1

:No1
  ECHO Need build version param (such as 7.2.1)
:End1