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

./sid2sapr -n $(((1*60+17)*50)) -b buzzy sid/'Freeze.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Freeze                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-mono
make player-mono
mv player.xex xex-mono/laxity-freeze.xex

./sid2sapr -n $(((2*60+22)*50)) -b buzzy sid/'Syncopated.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Syncopated                     " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-mono
make player-mono
mv player.xex xex-mono/laxity-syncopated.xex

./sid2sapr -n $(((1*60+51)*50)) -b buzzy sid/'Wisdom.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Wisdom                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-mono
make player-mono
mv player.xex xex-mono/laxity-wisdom.xex

#fi

# clear for further tests
echo -n " " > asm/songname.txt
