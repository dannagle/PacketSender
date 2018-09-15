#!/bin/bash

if [ -z "$1" ]
  then
    echo "Please supply build version (e.g. 5.8.2)"
    exit
fi

BUILD_VERSION="$1";


pushd /tmp/
rm -rf workspace || true
mkdir workspace
cd workspace
git clone https://github.com/dannagle/PacketSender
cd PacketSender/src
git checkout development

echo "Replacing globals.h with $BUILD_VERSION"
sed -i '' '/BEGIN/,/END/c\
#define SW_VERSION "v'$BUILD_VERSION'"
' globals.h

"/Users/dannagle/Qt/5.10.0/clang_64/bin/qmake" PacketSender.pro -spec macx-clang CONFIG+=x86_64
make
/Users/dannagle/Qt/5.10.0/clang_64/bin/macdeployqt PacketSender.app -appstore-compliant
codesign --deep --force --sign "Developer ID Application: NagleCode, LLC (C77T3Q8VPT)" PacketSender.app

rm -rf /Users/dannagle/github/PacketSender/PacketSender.app || true
mv PacketSender.app /Users/dannagle/github/PacketSender

rm -rf newbuild.dmg  || true
"/Applications/DMG Canvas.app/Contents/Resources/dmgcanvas" "/Users/dannagle/github/PacketSender/PacketSender.dmgCanvas" newbuild.dmg

rm -rf /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg || true
mv newbuild.dmg /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg

echo "Finished creating PacketSender_v$BUILD_VERSION.dmg"

popd
