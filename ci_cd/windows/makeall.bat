PUSHD %TMP%
rd /s /q PacketSender
git clone --depth 1 -b development git@github.com:dannagle/PacketSender.git
cd PacketSender
REM Setting Version Numbers
call ci_cd\windows\win-prebuild.bat
REM Building app
call ci_cd\windows\win-build.bat
REM Building installers
rem call ci_cd\windows\win-deploy.bat
POPD