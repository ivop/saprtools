#! /bin/sh

set -e

. ../player/convert.sh

create_sapr() {
  ./sid2sapr $stereo -b $basstype "$@" >>convertall.log 2>&1
}

echo >convertall.log

file_converter="sid2sapr"

for player in "stereo" "mono" "softbass" ; do
echo "Starting ${file_converter} conversion for ${player} player."

if [ "$player" = "mono" ]; then
    basstype=buzzy
else
    basstype=softbass
fi

if [ "$player" = "stereo" ]; then
    stereo=-s
else
    stereo=
fi

# Note: sid2sapr options "-b basstype" and "-a" are ignored in stereo mode

# if false; then

file_source="Commodore 64"

# JEROEN TEL
file_author="Jeroen Tel"

create_sapr -x 0 "sid/Alloyrun.sid"
create_title "Alloyrun" "tel-alloy"

create_sapr -d "sid/Alternative_Fuel.sid"
create_title "Alternative Fuel" "tel-fuel"

create_sapr -x 2 "sid/Cybernoid.sid"
create_title "Cybernoid" "tel-cybernoid"

create_sapr -x 1 -m both "sid/Cybernoid_II.sid"
create_title "Cybernoid II" "tel-cybernoid2"

create_sapr -x 2 -m ringmod "sid/Hawkeye.sid"
create_title "Hawkeye" "tel-hawkeye"

create_sapr -x 1 -m both "sid/Ice_Age.sid"
create_title "Ice Age" "tel-iceage"

create_sapr "sid/JT_42.sid"
create_title "JT 42" "tel-jt42"

create_sapr "sid/Kinetix.sid"
create_title "Kinetix" "tel-kinetix"

# LAXITY
file_author="Thomas E. Petersen (Laxity)"

create_sapr "sid/Freeze.sid"
create_title "Freeze" "laxity-freeze"

create_sapr "sid/Syncopated.sid"
create_title "Syncopated" "laxity-syncopated"

create_sapr "sid/Wisdom.sid"
create_title "Wisdom" "laxity-wisdom"

# ROB HUBBARD
file_author="Rob Hubbard"

create_sapr -x 0 "sid/International_Karate.sid"
export PLAYER_LOWMEM=-d:LOWMEM=0x1800
create_title "International Karate" "hubbard-ik"
unset PLAYER_LOWMEM

create_sapr "sid/Crazy_Comets.sid"
create_title "Crazy Comets" "hubbard-comets"

create_sapr "sid/Delta.sid"
create_title "Delta" "hubbard-delta"

create_sapr "sid/Lightforce.sid"
create_title "Lightforce" "hubbard-lightforce"

create_sapr "sid/Monty_on_the_Run.sid"
create_title "Monty on the Run" "hubbard-monty"

create_sapr "sid/Spellbound.sid"
create_title "Spellbound" "hubbard-spellbound"

create_sapr "sid/Thing_on_a_Spring.sid"
create_title "Thing On A Spring" "hubbard-toas"

create_sapr -d "sid/Warhawk.sid"
create_title "Warhawk" "hubbard-warhawk"

# MITCH & DANE
file_author="Mitch & Dane"

create_sapr "sid/Dose_of_D.sid"
create_title "Dose Of D" "md-dose"

create_sapr "sid/Jigsaw.sid"
create_title "Jigsaw" "md-jigsaw"

# CADAVER
file_author="Lasse Oorni (Cadaver)"

create_sapr "sid/Aces_High.sid"
create_title "Aces High" "cadaver-aceshigh"

# loop once
create_sapr -n $(((1*60+38)*50)) "sid/Escape_from_New_York.sid"
create_title "Escape from New York" "cadaver-escapefromny"

# loop once
create_sapr -n $(((1*60+18)*50)) "sid/GoatTracker_example_MW1_title.sid"
create_title "GoatTracker example MW1 title" "cadaver-goatexample"

create_sapr "sid/Metal_Warrior_4.sid"
create_title "Metal Warrior 4" "cadaver-mw4"

create_sapr "sid/Tarantula.sid"
create_title "Tarantula" "cadaver-tarantula"

create_sapr "sid/Unleash_the_Fucking_Fury.sid"
create_title "Unleash The Fucking Fury" "cadaver-unleash"

# MARTIN GALWAY
file_author="Martin Galway"

create_sapr "sid/Ocean_Loader_1.sid"
create_title "Ocean Loader 1" "galway-ocean1"

create_sapr "sid/Ocean_Loader_2.sid"
create_title "Ocean Loader 2" "galway-ocean2"

create_sapr -a "sid/Terra_Cresta.sid"
create_title "Terra Cresta" "galway-terra"

# PETER CLARKE
file_author="Peter Clarke"

create_sapr "sid/Ocean_Loader_3.sid"
create_title "Ocean Loader 3" "clarke-ocean3"

# CHRIS HUELSBECK
file_author="Chris Huelsbeck & Ramiro Vaca"

create_sapr "sid/R-Type.sid"
create_title "R-Type" "huelsbeck-rtype"

# CHARLES DEENEN
file_author="Charles Deenen"

create_sapr -m both -f "sid/Zamzara.sid"
create_title "Zamzara" "deenen-zamzara"

# NTSC VBI 60HZ TEST
file_author="Anthony Butch Davis (Deathlok)"

create_sapr -m all "sid/Snowflake.sid"
create_title "Snowflake" "deathlok-snowflake" "60"

# Original of 100HZ test cover
file_author="Martin Galway"

create_sapr -t2 -m all "sid/Rastan.sid"
create_title "Rastan (tune 2)" "galway-rastan-tune2"

# CIA 100HZ TEST (error in HSVC length database)
file_author="Kent Patfield (Patto)"

create_sapr -n 6500 -m all "sid/Rastan_Saga.sid"
create_title "Rastan Saga (cover)" "patto-rastan-tune2-cover" "100"

#fi

done

# clear for further tests
rm songname.txt

# remove one that's too big (53k)
rm "../xex/${file_converter}/stereo/hubbard-ik.xex"
