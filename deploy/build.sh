#! /bin/sh

# RUN ME FROM THE ROOT DIRECTORY OF THE POJECT, LIKE:
#
# deploy/build.sh

BASE=`pwd`
DEPLOY="$BASE"/deploy

echo "BUILDING SAPRTOOLS"

make distclean
make -j`nproc` all STATIC="-static -s" CC="${DEPLOY_CC}" CXX="${DEPLOY_CXX}"

echo "COLLECT FILES FOR DISTRIBUTION"

rm -rf "$COLLECT"
mkdir -p "$COLLECT"

for i in `cat "$DEPLOY/files.txt"` ; do
    j="$i"${EXE}
    test -n "${MSYSTEM}" -a "$j" = "sndh2ym/sndh2ym.exe" || \
        cp -a "$j" "$COLLECT"
done
