#! /bin/sh

# RUN ME FROM THE ROOT DIRECTORY OF THE POJECT, LIKE:
#
# deploy/build.sh

BASE=`pwd`
DEPLOY="$BASE"/deploy

printf "\nBUILDING SAPRTOOLS\n\n"

make distclean
make -j`nproc` all \
    STATIC="$STATIC" \
    CC="${DEPLOY_CC}" \
    CXX="${DEPLOY_CXX}" \
    AR="${DEPLOY_AR}" \
    RANLIB="${DEPLOY_RANLIB}"

printf "\nCOLLECT FILES FOR DISTRIBUTION\n\n"

rm -rf "$COLLECT"
mkdir -p "$COLLECT"

for i in `cat "$DEPLOY/files.txt"` ; do
    j="$i"${EXE}
    test -n "${MSYSTEM}" -a "$j" = "sndh2ym/sndh2ym.exe" || \
        cp -a "$j" "$COLLECT"
done
