#!/usr/bin/env bash

if [ -z "$1" ]
  then
    echo "Please supply build version (e.g. 8.0.3)"
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

# "/Users/dannagle/Qt/6.5.2/macos/bin/qmake" PacketSender.pro -spec macx-clang CONFIG+=qtquickcompiler
"/Users/dannagle/Qt/5.15.2/clang_64/bin/qmake" PacketSender.pro -spec macx-clang CONFIG+=x86_64
make
# /Users/dannagle/Qt/6.5.2/macos/bin/macdeployqt packetsender.app -appstore-compliant
/Users/dannagle/Qt/5.15.2/clang_64/bin/macdeployqt packetsender.app -appstore-compliant

/usr/bin/codesign --option runtime --deep --force --sign  38E41C6C66CCA827750A10E26539E038F033E933 --timestamp packetsender.app
mv packetsender.app PacketSender.app
rm -rf /Users/dannagle/github/PacketSender/PacketSender.app || true
mv PacketSender.app /Users/dannagle/github/PacketSender

rm -rf newbuild.dmg  || true
"/Applications/DMG Canvas.app/Contents/Resources/dmgcanvas" "/Users/dannagle/github/PacketSender/PacketSender.dmgCanvas" newbuild.dmg -identity  38E41C6C66CCA827750A10E26539E038F033E933 -notarizationPrimaryBundleID "com.packetsender.desktop" -identity "38E41C6C66CCA827750A10E26539E038F033E933" -notarizationAppleID "$2" -notarizationPassword "$3"

rm -rf /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg || true
mv newbuild.dmg /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg

echo "Finished creating PacketSender_v$BUILD_VERSION.dmg"

#echo "Sending to Apple for notary"
#xcrun altool --notarize-app -f /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg --primary-bundle-id 'com.packetsender.desktop'  -u ''$APPLE_UNAME'' -p ''$APPLE_PWORD''


# xcrun altool --notarize-app -f _dmg_ --primary-bundle-id 'com.packetsender.desktop'  -u "_email_" -p "app_code" --output-format xml
# /usr/bin/xcrun altool --notarization-info notary_id -u _email_ --output-format xml

popd
