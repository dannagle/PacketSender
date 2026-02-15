#!/usr/bin/env bash

if [ -z "$1" ]
  then
    echo "Please supply build version (e.g. 8.9.2)"
    exit
fi

if [ -z "$2" ]
  then
    echo "Please supply notary id username (e.g. apple@example.com)"
    exit
fi

if [ -z "$3" ]
  then
    echo "Please supply appleid application password (e.g. hunter2)"
    exit
fi


BUILD_VERSION="$1";
APPLE_UNAME="$2";
APPLE_PWORD="$3";


pushd $TMPDIR
rm -rf workspace || true
mkdir workspace
cd workspace
# git clone --depth 1 -b development git@github.com:dannagle/PacketSender.git
git clone /Users/dannagle/github/PacketSender

cd PacketSender/src
git checkout development

echo "Replacing globals.h with $BUILD_VERSION"
sed -i '' '/BEGIN/,/END/c\
#define SW_VERSION "v'$BUILD_VERSION'"
' globals.h

echo "Replacing Info.plist with $BUILD_VERSION"
sed -i '' 's/<string>1.0<\/string>/<string>'$BUILD_VERSION'<\/string>/' Info.plist

# "/Users/dannagle/Qt/6.9.0/macos/bin/qmake" PacketSender.pro -spec macx-clang CONFIG+=qtquickcompiler
"/Users/dannagle/Qt/6.9.0/macos/bin/qmake" PacketSender.pro -spec macx-clang CONFIG+=qtquickcompiler QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64"

# /Users/dannagle/Qt/6.9.0/macos/bin/qmake /Users/dannagle/github/PacketSender/src/PacketSender.pro -spec macx-clang CONFIG+=qtquickcompiler QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" && /usr/bin/make qmake_all

make 
# /Users/dannagle/Qt/6.9.0/macos/bin/macdeployqt packetsender.app -appstore-compliant
/Users/dannagle/Qt/6.9.0/macos/bin/macdeployqt packetsender.app -appstore-compliant

/usr/bin/codesign --option runtime --deep --force --sign  617D1C7451C6C9DF68026C4F337DF11209FC8A14 --timestamp packetsender.app
mv packetsender.app PacketSender.app
rm -rf /Users/dannagle/github/PacketSender/PacketSender.app || true
mv PacketSender.app /Users/dannagle/github/PacketSender

rm -rf newbuild.dmg  || true
"/Applications/DMG Canvas.app/Contents/Resources/dmgcanvas" "/Users/dannagle/github/PacketSender/PacketSender.dmgCanvas" newbuild.dmg 
# -identity  617D1C7451C6C9DF68026C4F337DF11209FC8A14 -notarizationPrimaryBundleID "com.packetsender.desktop" -identity "617D1C7451C6C9DF68026C4F337DF11209FC8A14" -notarizationAppleID "$2" -notarizationPassword "$3"

rm -rf /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg || true
mv newbuild.dmg /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg

echo "Finished creating PacketSender_v$BUILD_VERSION.dmg"

#echo "Sending to Apple for notary"
#xcrun altool --notarize-app -f /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg --primary-bundle-id 'com.packetsender.desktop'  -u ''$APPLE_UNAME'' -p ''$APPLE_PWORD''


# xcrun altool --notarize-app -f _dmg_ --primary-bundle-id 'com.packetsender.desktop'  -u "_email_" -p "app_code" --output-format xml
# /usr/bin/xcrun altool --notarization-info notary_id -u _email_ --output-format xml

popd
