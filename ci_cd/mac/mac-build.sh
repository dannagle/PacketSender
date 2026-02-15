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

export CMAKE_PREFIX_PATH=~/Qt/6.9.2/macos/lib/cmake

mkdir build
cd build

~/Qt/Tools/CMake/CMake.app/Contents/bin/cmake -G Xcode \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_PREFIX_PATH=~/Qt/6.9.2/macos/lib/cmake \
  -DCMAKE_INSTALL_PREFIX=~/QtBuilds/install \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DQT_QMAKE_EXECUTABLE=~/Qt/6.9.2/macos/bin/qmake \
  -DQT_DEBUG_FIND_PACKAGE=ON \
  -DQT_NO_CREATE_VERSIONLESS_TARGETS=OFF \
  -DQT_FEATURE_webengine=ON \
  -DQT_FEATURE_multimedia=ON \
  -DQT_FEATURE_network=ON \
  -DQT_FEATURE_widgets=ON \
  -DQT_FEATURE_gui=ON \
  -DQT_FEATURE_core=ON \
  ..

rm -rf /Users/dannagle/github/PacketSender/PacketSender.app || true


~/Qt/Tools/CMake/CMake.app/Contents/bin/cmake --build . --config Release


~/Qt/6.9.2/macos/bin/macdeployqt Release/packetsender.app  -appstore-compliant

mv Release/packetsender.app packetsender.app

/usr/bin/codesign --option runtime --deep --force --sign  78011CB7EB94BAD1766EF1B6BF6C9A132F6D0571 --timestamp packetsender.app
mv packetsender.app PacketSender.app
mv PacketSender.app /Users/dannagle/github/PacketSender

rm -rf newbuild.dmg  || true
"/Applications/DMG Canvas.app/Contents/Resources/dmgcanvas" "/Users/dannagle/github/PacketSender/PacketSender.dmgCanvas" newbuild.dmg 
# -identity  78011CB7EB94BAD1766EF1B6BF6C9A132F6D0571 -notarizationPrimaryBundleID "com.packetsender.desktop" -identity "78011CB7EB94BAD1766EF1B6BF6C9A132F6D0571" -notarizationAppleID "$2" -notarizationPassword "$3"

rm -rf /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg || true
mv newbuild.dmg /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg

echo "Finished creating PacketSender_v$BUILD_VERSION.dmg"

#echo "Sending to Apple for notary"
#xcrun altool --notarize-app -f /Users/dannagle/github/PacketSender/PacketSender_v$BUILD_VERSION.dmg --primary-bundle-id 'com.packetsender.desktop'  -u ''$APPLE_UNAME'' -p ''$APPLE_PWORD''


# xcrun altool --notarize-app -f _dmg_ --primary-bundle-id 'com.packetsender.desktop'  -u "_email_" -p "app_code" --output-format xml
# /usr/bin/xcrun altool --notarization-info notary_id -u _email_ --output-format xml

popd
