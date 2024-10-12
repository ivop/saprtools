#! /bin/sh

# RUN ME FROM THE ROOT DIRECTORY OF THE POJECT, LIKE:
#
# deploy/deploy-all.sh
#
# Cygwin cross-compilation via cygcross (https://github.com/ivop/cygcross)
# Make sure all compilers are in your PATH!
# For cygcross, use ..../cygcross/sysroot/usr/bin

set -e

for i in linux32 linux64 cygwin64 mingw32 mingw64 ; do
    deploy/deploy.sh "$i"
done
