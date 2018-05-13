!#/bin/bash

pushd /tmp/
rm -rf workspace || true
mkdir workspace
cd workspace
git clone https://github.com/dannagle/PacketSender
cd PacketSender/src
git checkout development
"/Users/dannagle/Qt/5.10.0/clang_64/bin/qmake" PacketSender.pro -spec macx-clang CONFIG+=x86_64
make
/Users/dannagle/Qt/5.10.0/clang_64/bin/macdeployqt PacketSender.app -appstore-compliant
codesign --deep --force --sign "Developer ID Application: NagleCode, LLC (C77T3Q8VPT)" PacketSender.app

rm -rf /Users/dannagle/github/PacketSender/PacketSender.app || true
mv PacketSender.app /Users/dannagle/github/PacketSender

rm -rf newbuild.dmg  || true
"/Applications/DMG Canvas.app/Contents/Resources/dmgcanvas" "/Users/dannagle/github/PacketSender/PacketSender.dmgCanvas" newbuild.dmg 

rm -rf /Users/dannagle/github/PacketSender/PacketSender_ready.dmg || true
mv newbuild.dmg /Users/dannagle/github/PacketSender/PacketSender_ready.dmg

popd
