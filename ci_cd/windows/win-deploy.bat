echo Building Installers
cd C:\Users\Dan\github\packetsenderinstaller
"C:\Program Files\7-Zip\7z.exe" a PacketSenderPortable-Release.zip PacketSenderPortable
move PacketSenderPortable-Release.zip installer
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" packetsender32bit.iss
cd installer
copy /Y C:\Users\Dan\Desktop\code_sign_exe_com.bat .
call code_sign_exe_com.bat
del code_sign_exe_com.bat
