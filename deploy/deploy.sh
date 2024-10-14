#! /bin/sh

# RUN ME FROM THE ROOT DIRECTORY OF THE POJECT, LIKE:
#
# deploy/deploy.sh

set -e

DATE=$(date "+%Y%m%d")

BASE=`pwd`
DEPLOY="$BASE"/deploy
ASSETS="$DEPLOY"/assets
COLLECT="$DEPLOY/saprtools-$DATE"

if [ "$#" != "1" ] ; then
    cat <<EOT
no target specified
usage: $0 target
where target is one of: linux32, linux64, cygwin64, mingw32 or mingw64
EOT
    exit 1
fi

unset MSYSTEM
unset EXE
unset ZIP
unset DMG
unset CYGDLL
STATIC="-static -s"
DEPLOY_AR=ar
DEPLOY_RANLIB=ranlib

case "$1" in
    linux32)
        DEPLOY_CC="x86_64-linux-gnu-gcc -m32"
        DEPLOY_CXX="x86_64-linux-gnu-g++ -m32"
        MADS=mads-bullseye-i386
        CONVERT=convert.sh
        ;;
    linux64)
        DEPLOY_CC=x86_64-linux-gnu-gcc
        DEPLOY_CXX=x86_64-linux-gnu-g++
        MADS=mads-bullseye-x86_64
        CONVERT=convert.sh
        ;;
    cygwin64)
        DEPLOY_CC=x86_64-pc-cygwin-gcc
        DEPLOY_CXX=x86_64-pc-cygwin-g++
        MADS=mads-win64.exe
        CONVERT=convert.bat
        EXE=.exe
        ZIP=true
        CYGDLL=true
        ;;
    mingw32)
        DEPLOY_CC=i686-w64-mingw32-gcc
        DEPLOY_CXX=i686-w64-mingw32-g++
        MADS=mads-win32.exe
        CONVERT=convert.bat
        EXE=.exe
        ZIP=true
        MSYSTEM=mingw32
        ;;
    mingw64)
        DEPLOY_CC=x86_64-w64-mingw32-gcc
        DEPLOY_CXX=x86_64-w64-mingw32-g++
        MADS=mads-win64.exe
        CONVERT=convert.bat
        EXE=.exe
        ZIP=true
        MSYSTEM=mingw64
        ;;
    macos-x86_64)
        DEPLOY_CC=x86_64-apple-darwin20.2-cc
        DEPLOY_CXX=x86_64-apple-darwin20.2-c++
        MADS=mads-macos-x86_64
        CONVERT=convert.sh
        DEPLOY_AR=x86_64-apple-darwin20.2-ar
        DEPLOY_RANLIB=x86_64-apple-darwin20.2-ranlib
        STATIC=
        DMG=true
        ;;
    macos-arm64)
        DEPLOY_CC=arm64-apple-darwin20.2-cc
        DEPLOY_CXX=arm64-apple-darwin20.2-c++
        MADS=mads-macos-arm64
        CONVERT=convert.sh
        DEPLOY_AR=arm64-apple-darwin20.2-ar
        DEPLOY_RANLIB=arm64-apple-darwin20.2-ranlib
        STATIC=
        DMG=true
        ;;
    *)
        echo "unknown target '$1'"
        exit 1
        ;;
esac

export DEPLOY_CC DEPLOY_CXX MSYSTEM EXE COLLECT STATIC DEPLOY_AR DEPLOY_RANLIB
export MACOSX_DEPLOYMENT_TARGET=10.15
deploy/build.sh

cp -a player/asm "$COLLECT"
cp -a sid2sapr/sid/Hawkeye.sid "$COLLECT"
cp -a "$ASSETS/$MADS" "$COLLECT/mads$EXE"
cp -a "$ASSETS/$CONVERT" "$COLLECT"
cp -a "$ASSETS/songname.txt" "$COLLECT"

if [ -n "$CYGDLL" ] ; then
    A=$(which $DEPLOY_CC)
    A=${A%/*}
    cp -a "$A/../usr/bin/cygwin1.dll" "$COLLECT"
fi

if [ -n "$ZIP" ] ; then
    cd "$COLLECT"
    zip -9r "$BASE/saprtools-$1-$DATE.zip" *
elif [ -n "$DMG" ] ; then
    genisoimage -r -apple -probe -o saprtools-$1-$DATE.iso "$COLLECT"
    dmg saprtools-$1-$DATE.iso saprtools-$1-$DATE.dmg
    rm -f saprtools-$1-$DATE.iso
else
    tar cvzf saprtools-$1-$DATE.tar.gz -C "$DEPLOY" "$(basename "$COLLECT")"
    tar cvjf saprtools-$1-$DATE.tar.bz2 -C "$DEPLOY" "$(basename "$COLLECT")"
    tar cvJf saprtools-$1-$DATE.tar.xz -C "$DEPLOY" "$(basename "$COLLECT")"
fi
rm -rf "$COLLECT"
