#! /bin/sh

set -e

make

for player in mono softbass ; do

if [ "$player" = "mono" ]; then
    basstype=buzzy
else
    basstype=softbass
fi

./sid2sapr -n $(((6*60+40)*50)) -b $basstype sid/'Cybernoid.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Cybernoid                      " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/tel-cybernoid.xex

./sid2sapr -a -n $(((5*60+46)*50)) -b $basstype sid/'Cybernoid_II.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Cybernoid II                   " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/tel-cybernoid2.xex

./sid2sapr -n $(((1*60+17)*50)) -b $basstype sid/'Freeze.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Freeze                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/laxity-freeze.xex

./sid2sapr -n $(((2*60+22)*50)) -b $basstype sid/'Syncopated.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Syncopated                     " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/laxity-syncopated.xex

./sid2sapr -n $(((1*60+51)*50)) -b $basstype sid/'Wisdom.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Wisdom                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/laxity-wisdom.xex

./sid2sapr -n $(((10*60+45)*50)) -b $basstype sid/'International_Karate.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : International Karate           " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/hubbard-ik.xex

./sid2sapr -n $(((1*60+5)*50)) -b $basstype sid/'Dose_of_D.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Dose Of D                      " \
    "Author: Mitch & Dane" > asm/songname.txt
make compress-$player
make player-$player
mv player.xex xex-$player/md-dose.xex

done

# clear for further tests
echo -n " " > asm/songname.txt
