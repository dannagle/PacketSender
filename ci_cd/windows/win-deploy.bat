echo Building Installers
set /p BUILD_VERSION=<buildversion.txt
set "BUILD_VERSION=%BUILD_VERSION: =%"
cd C:\Users\Dan\github\packetsenderinstaller\installer
move *.* ../archive
cd ..

"C:\Program Files\7-Zip\7z.exe" a PacketSenderPortable_v%BUILD_VERSION%.zip PacketSenderPortable
move PacketSenderPortable_v%BUILD_VERSION%.zip installer
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" packetsender64bit.iss
cd installer
copy /Y C:\Users\Dan\Desktop\code_sign_exe_com.bat .
call code_sign_exe_com.bat
del code_sign_exe_com.bat
