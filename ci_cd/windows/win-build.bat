echo Making exe
C:\Qt\5.10.1\mingw53_32\bin\qmake.exe -o Makefile src/PacketSender.pro -spec win32-g++
C:\Qt\Tools\mingw530_32\bin\mingw32-make.exe -f Makefile.Release

echo Signing exe
cd release
copy /Y C:\Users\Dan\Desktop\code_sign_exe_com.bat .
call code_sign_exe_com.bat
echo Copying signed exe
copy /Y PacketSender.exe C:\Users\Dan\github\packetsenderinstaller\q5mingw5.10\PacketSender.exe
copy /Y PacketSender.exe C:\Users\Dan\github\packetsenderinstaller\PacketSenderPortable\PacketSender.exe
cd ..

echo Cleaning build
rd /s /q release

echo Making command line exe
C:\Qt\5.10.1\mingw53_32\bin\qmake.exe -o Makefile src/PacketSenderCLI.pro -spec win32-g++
C:\Qt\Tools\mingw530_32\bin\mingw32-make.exe -f Makefile.Release

echo Signing command line exe
cd release
copy /Y C:\Users\Dan\Desktop\code_sign_exe_com.bat .
call code_sign_exe_com.bat
echo Copying command line exe
copy /Y packetsendercli.exe C:\Users\Dan\github\packetsenderinstaller\q5mingw5.10\packetsender.com
copy /Y packetsendercli.exe C:\Users\Dan\github\packetsenderinstaller\PacketSenderPortable\packetsender.com
cd ..
