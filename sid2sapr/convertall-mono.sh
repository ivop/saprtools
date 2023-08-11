#! /bin/sh

set -e

make

#if false; then

./sid2sapr -n $(((6*60+40)*50)) -b buzzy sid/'Cybernoid.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Cybernoid                      " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress-mono
make player-mono
mv player.xex xex-mono/tel-cybernoid.xex

./sid2sapr -n $(((5*60+46)*50)) -b buzzy sid/'Cybernoid_II.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Cybernoid II                   " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress-mono
make player-mono
mv player.xex xex-mono/tel-cybernoid2.xex

./sid2sapr -n $(((1*60+17)*50)) -b buzzy sid/'freeze.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Freeze                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-mono
make player-mono
mv player.xex xex-mono/laxity-freeze.xex

#fi

# clear for further tests
echo -n " " > asm/songname.txt
