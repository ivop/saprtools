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
unset CYGDLL

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
    *)
        echo "unknown target '$1'"
        exit 1
        ;;
esac

export DEPLOY_CC DEPLOY_CXX MSYSTEM EXE COLLECT
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
else
    tar cvzf saprtools-$1-$DATE.tar.gz -C "$DEPLOY" "$(basename "$COLLECT")"
fi
rm -rf "$COLLECT"
