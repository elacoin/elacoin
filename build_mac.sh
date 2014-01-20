#/usr/bin/env bash
# You will need to change these paths to the right ones for your system
qmake "USE_QRCODE=1" \
	"BOOST_INCLUDE_PATH=/usr/local/include" \
	"BOOST_LIB_PATH=/usr/local/Cellar/boost/1.55.0/lib" \
	"MINIUPNPC_LIB_PATH=/usr/local/Cellar/miniupnpc/1.8.20131007/lib" \
	"BDB_LIB_PATH=/usr/local/Cellar/berkeley-db4/4.8.30/lib/" \
	"BDB_INCLUDE_PATH=/usr/local/Cellar/berkeley-db4/4.8.30/include" \
	"QRENCODE_LIB_PATH=/usr/local/Cellar/qrencode/3.4.3/lib" \
	"RELEASE=1" \
	elacoin-qt.pro

# create dummy build.h until I figure out why xcodebuild wants it
touch src/build.h
xcodebuild -target Elacoin-Qt -configuration Release
rm src/build.h
cd build/Release
../../contrib/macdeploy/macdeployqtplus Elacoin-Qt.app -add-qt-tr da,de,es,sv,hu,ru,uk,zh_CN,zh_TW -dmg -verbose 2 -fancy ../../contrib/macdeploy/fancy.plist
cd ../../
