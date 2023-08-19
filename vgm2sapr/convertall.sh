#! /bin/sh

set -e

make

#if false; then

./vgm2sapr bbc/CrazeeRider-title.vgz
echo -n \
    "Source: BBC Micro                      " \
    "Title : Crazee Rider                   " \
    "Author: Martin Galway" > asm/songname.txt
make compress
make player
mv player.xex xex/bbc-crazeerider.xex

./vgm2sapr bbc/Galaforce2-title.vgz
echo -n \
    "Source: BBC Micro                      " \
    "Title : Galaforce 2                    " \
    "Author: Martin Galway" > asm/songname.txt
make compress
make player
mv player.xex xex/bbc-galaforce2.xex

./vgm2sapr bbc/Firetrack-ingame.vgz
echo -n \
    "Source: BBC Micro                      " \
    "Title : Firetrack                      " \
    "Author: Nick Pelling" > asm/songname.txt
make compress
make player
mv player.xex xex/bbc-firetrack.xex

./vgm2sapr bbc/TubularBells.vgz
echo -n \
    "Source: BBC Micro                      " \
    "Title : Tubular Bells (demo)           " \
    "Author: Nigel Scott" > asm/songname.txt
make compress
make player
mv player.xex xex/bbc-tubular.xex

#fi

# clear for further tests
echo -n " " > asm/songname.txt
