#! /bin/sh

# RUN ME FROM THE ROOT DIRECTORY OF THE POJECT, LIKE:
#
# deploy/deploy-all.sh
#
# Cygwin cross-compilation cygcross (https://github.com/ivop/cygcross)
# macOS cross-compilation osxcross (https://github.com/tpoechtrager/osxcross)
# (XCode 12.4)
# DMG with genisoimage and dmg (https://github.com/planetbeing/libdmg-hfsplus)
#
# Make sure all compilers and tools are in your PATH!
# For cygcross, use ..../cygcross/sysroot/usr/bin

set -e

for i in linux32 linux64 cygwin64 mingw32 mingw64 macos-x86_64 macos-arm64 ; do
    deploy/deploy.sh "$i"
done
