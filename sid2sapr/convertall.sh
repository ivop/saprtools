#! /bin/sh

set -e

make

for player in "" -mono -softbass ; do

if [ "$player" = "-mono" ]; then
    basstype=buzzy
else
    basstype=softbass
fi

if [ "$player" = "" ]; then
    stereo=-s
else
    stereo=
fi

# -b basstype and -a are ignored in stereo mode

#if false; then

# JEROEN TEL

./sid2sapr $stereo -x 0 -b $basstype sid/'Alloyrun.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Alloyrun                       " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-alloy.xex

./sid2sapr $stereo -d -b $basstype sid/'Alternative_Fuel.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Alternative Fuel               " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-fuel.xex

./sid2sapr $stereo -x 2 -b $basstype sid/'Cybernoid.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Cybernoid                      " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-cybernoid.xex

./sid2sapr $stereo -x 1 -m both -b $basstype sid/'Cybernoid_II.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Cybernoid II                   " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-cybernoid2.xex

./sid2sapr $stereo -x 2 -m ringmod -b $basstype sid/'Hawkeye.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Hawkeye                        " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-hawkeye.xex

./sid2sapr $stereo -x 1 -m both -b $basstype sid/'Ice_Age.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Ice Age                        " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-iceage.xex

./sid2sapr $stereo -b $basstype sid/'JT_42.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : JT 42                          " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-jt42.xex

./sid2sapr $stereo -b $basstype sid/'Kinetix.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Kinetix                        " \
    "Author: Jeroen Tel" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/tel-kinetix.xex

# LAXITY

./sid2sapr $stereo -b $basstype sid/'Freeze.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Freeze                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/laxity-freeze.xex

./sid2sapr $stereo -b $basstype sid/'Syncopated.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Syncopated                     " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/laxity-syncopated.xex

./sid2sapr $stereo -b $basstype sid/'Wisdom.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Wisdom                         " \
    "Author: Thomas E. Petersen (Laxity)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/laxity-wisdom.xex

# ROB HUBBARD

./sid2sapr $stereo -x 0 -b $basstype sid/'International_Karate.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : International Karate           " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player PLAYER_LOWMEM=-d:LOWMEM=0x1800
mv player.xex xex$player/hubbard-ik.xex

./sid2sapr $stereo -b $basstype sid/'Crazy_Comets.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Crazy Comets                   " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-comets.xex

./sid2sapr $stereo -b $basstype sid/'Delta.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Delta                          " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-delta.xex

./sid2sapr $stereo -b $basstype sid/'Lightforce.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Lightforce                     " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-lightforce.xex

./sid2sapr $stereo -b $basstype sid/'Monty_on_the_Run.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Monty on the Run               " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-monty.xex

./sid2sapr $stereo -b $basstype sid/'Spellbound.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Spellbound                     " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-spellbound.xex

./sid2sapr $stereo -b $basstype sid/'Thing_on_a_Spring.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Thing On A Spring              " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-toas.xex

./sid2sapr $stereo -d -b $basstype sid/'Warhawk.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Warhawk                        " \
    "Author: Rob Hubbard" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/hubbard-warhawk.xex

# MITCH & DANE

./sid2sapr $stereo -b $basstype sid/'Dose_of_D.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Dose Of D                      " \
    "Author: Mitch & Dane" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/md-dose.xex

./sid2sapr $stereo -b $basstype sid/'Jigsaw.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Jigsaw                         " \
    "Author: Mitch & Dane" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/md-jigsaw.xex

# CADAVER

./sid2sapr $stereo -b $basstype sid/'Aces_High.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Aces High                      " \
    "Author: Lasse Oorni (Cadaver)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/cadaver-aceshigh.xex

# loop once
./sid2sapr $stereo -n $(((1*60+38)*50)) -b $basstype sid/'Escape_from_New_York.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Escape from New York           " \
    "Author: Lasse Oorni (Cadaver)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/cadaver-escapefromny.xex

# loop once
./sid2sapr $stereo -n $(((1*60+18)*50)) -b $basstype sid/'GoatTracker_example_MW1_title.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : GoatTracker example MW1 title  " \
    "Author: Lasse Oorni (Cadaver)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/cadaver-goatexample.xex

./sid2sapr $stereo -b $basstype sid/'Metal_Warrior_4.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Metal Warrior 4                " \
    "Author: Lasse Oorni (Cadaver)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/cadaver-mw4.xex

./sid2sapr $stereo -b $basstype sid/'Tarantula.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Tarantula                      " \
    "Author: Lasse Oorni (Cadaver)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/cadaver-tarantula.xex

./sid2sapr $stereo -b $basstype sid/'Unleash_the_Fucking_Fury.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Unleash The Fucking Fury       " \
    "Author: Lasse Oorni (Cadaver)" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/cadaver-unleash.xex

# MARTIN GALWAY

./sid2sapr $stereo -b $basstype sid/'Ocean_Loader_1.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Ocean Loader 1                 " \
    "Author: Martin Galway" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/galway-ocean1.xex

./sid2sapr $stereo -b $basstype sid/'Ocean_Loader_2.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Ocean Loader 2                 " \
    "Author: Martin Galway" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/galway-ocean2.xex

./sid2sapr -a $stereo -b $basstype sid/'Terra_Cresta.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Terra Cresta                   " \
    "Author: Martin Galway" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/galway-terra.xex

# PETER CLARKE

./sid2sapr $stereo -b $basstype sid/'Ocean_Loader_3.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Ocean Loader 3                 " \
    "Author: Peter Clarke" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/clarke-ocean3.xex

# CHRIS HUELSBECK

./sid2sapr $stereo -b $basstype sid/'R-Type.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : R-Type                         " \
    "Author: Chris Huelsbeck & Ramiro Vaca" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/huelsbeck-rtype.xex

# CHARLES DEENEN

./sid2sapr $stereo -m both -f -b $basstype sid/'Zamzara.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Zamzara                        " \
    "Author: Charles Deenen" > asm/songname.txt
make compress$player
make player$player
mv player.xex xex$player/deenen-zamzara.xex

# NTSC VBI 60HZ TEST

./sid2sapr $stereo -m all -b $basstype sid/'Snowflake.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Snowflake                      " \
    "Author: Anthony Butch Davis (Deathlok)" > asm/songname.txt
make compress$player player60$player
mv player.xex xex$player/deathlok-snowflake.xex

# Original of 100HZ test cover

./sid2sapr $stereo -t2 -m all -b $basstype sid/'Rastan.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Rastan (tune 2)                " \
    "Author: Martin Galway" > asm/songname.txt
make compress$player player$player
mv player.xex xex$player/galway-rastan-tune2.xex

# CIA 100HZ TEST (error in HSVC length database)

./sid2sapr $stereo -n 6500 -m all -b $basstype sid/'Rastan_Saga.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Rastan Saga (cover)            " \
    "Author: Kent Patfield (Patto)" > asm/songname.txt
make compress$player player100$player
mv player.xex xex$player/patto-rastan-tune2-cover.xex

#fi

done

# clear for further tests
echo -n " " > asm/songname.txt

# remove one that's too big
rm xex/hubbard-ik.xex
