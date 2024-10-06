#! /bin/sh

set -e
make all
mkdir -p ym

for i in sndh/* ; do
    j=ym/`basename "$i" .sndh`.ym
    echo "Converting '$i' to '$j'"
    ./sndh2ym -o "$j" "$i"
done
