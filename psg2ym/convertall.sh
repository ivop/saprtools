#! /bin/sh

set -e
make all

mkdir -p ym

for i in psg/*.psg ; do
    j=ym/`basename "$i" .psg`.ym
    echo "Converting '$i' to '$j'"
    ./psg2ym "$i" "$j"
done
