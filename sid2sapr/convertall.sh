#! /bin/zsh

set -e

# Parameters: file_source file_author_file_title file_name
function create_output(){
  file_source=$1
  file_author=$2
  file_title=$3
  file_name=$4.xex
  echo "Creating title $file_title by $file_author from $file_source as $file_name with player $player".
  make compress$player

  printf 'Source: %-32sTitle : %-32sAuthor: %s' "$file_source" "$file_title" "$file_author" > asm/songname.txt

  make player50$player
  mv player.xex xex$player/$file_name
}

# Evironment: file_source file_author
# Parameters: file_title file_name
function create_title(){
  create_output "$file_source" "$file_author" $1 $2
}

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

file_source="Commodore 64"

# JEROEN TEL
file_author="Jeroen Tel"

./sid2sapr $stereo -x 0 -b $basstype sid/'Alloyrun.sid'
create_title "Alloyrun" "tel-alloy"

./sid2sapr $stereo -d -b $basstype sid/'Alternative_Fuel.sid'
create_title "Alternative Fuel" "tel-fuel"

./sid2sapr $stereo -x 2 -b $basstype sid/'Cybernoid.sid'
create_title "Cybernoid" "tel-cybernoid"

./sid2sapr $stereo -x 1 -m both -b $basstype sid/'Cybernoid_II.sid'
create_title "Cybernoid II" "tel-cybernoid2"

./sid2sapr $stereo -x 2 -m ringmod -b $basstype sid/'Hawkeye.sid'
create_title "Hawkeye" "tel-hawkeye"

./sid2sapr $stereo -x 1 -m both -b $basstype sid/'Ice_Age.sid'
create_title "Ice Age" "tel-iceage"

./sid2sapr $stereo -b $basstype sid/'JT_42.sid'
create_title "JT 42" "tel-jt42"

./sid2sapr $stereo -b $basstype sid/'Kinetix.sid'
create_title "Kinetix" "tel-kinetix"

# LAXITY
file_author="Thomas E. Petersen (Laxity)"

./sid2sapr $stereo -b $basstype sid/'Freeze.sid'
create_title "Freeze" "laxity-freeze"

./sid2sapr $stereo -b $basstype sid/'Syncopated.sid'
create_title "Syncopated" "laxity-syncopated"

./sid2sapr $stereo -b $basstype sid/'Wisdom.sid'
create_title "Wisdom" "laxity-wisdom"


# ROB HUBBARD
file_author="Rob Hubbard"

./sid2sapr $stereo -x 0 -b $basstype sid/'International_Karate.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : International Karate           " \
    "Author: $file_author" > asm/songname.txt
make compress$player
make player$player PLAYER_LOWMEM=-d:LOWMEM=0x1800
mv player.xex xex$player/hubbard-ik.xex

./sid2sapr $stereo -b $basstype sid/'Crazy_Comets.sid'
create_title "Crazy Comets" "hubbard-comets"

./sid2sapr $stereo -b $basstype sid/'Delta.sid'
create_title "Delta" "hubbard-delta"

./sid2sapr $stereo -b $basstype sid/'Lightforce.sid'
create_title "Lightforce" "hubbard-lightforce"

./sid2sapr $stereo -b $basstype sid/'Monty_on_the_Run.sid'
create_title "Monty on the Run" "hubbard-monty"

./sid2sapr $stereo -b $basstype sid/'Spellbound.sid'
create_title "Spellbound" "hubbard-spellbound"

./sid2sapr $stereo -b $basstype sid/'Thing_on_a_Spring.sid'
create_title "Thing On A Spring" "hubbard-toas"

./sid2sapr $stereo -d -b $basstype sid/'Warhawk.sid'
create_title "Warhawk" "hubbard-warhawk"

# MITCH & DANE
file_author="Mitch & Dane"

./sid2sapr $stereo -b $basstype sid/'Dose_of_D.sid'
create_title "Dose Of D" "md-dose"

./sid2sapr $stereo -b $basstype sid/'Jigsaw.sid'
create_title "Jigsaw" "md-jigsaw"

# CADAVER
file_author="Lasse Oorni (Cadaver)"

./sid2sapr $stereo -b $basstype sid/'Aces_High.sid'
create_title "Aces High" "cadaver-aceshigh"

# loop once
./sid2sapr $stereo -n $(((1*60+38)*50)) -b $basstype sid/'Escape_from_New_York.sid'
create_title "Escape from New York" "cadaver-escapefromny"

# loop once
./sid2sapr $stereo -n $(((1*60+18)*50)) -b $basstype sid/'GoatTracker_example_MW1_title.sid'
create_title "GoatTracker example MW1 title" "cadaver-goatexample"

./sid2sapr $stereo -b $basstype sid/'Metal_Warrior_4.sid'
create_title "Metal Warrior 4" "cadaver-mw4"

./sid2sapr $stereo -b $basstype sid/'Tarantula.sid'
create_title "Tarantula" "cadaver-tarantula"

./sid2sapr $stereo -b $basstype sid/'Unleash_the_Fucking_Fury.sid'
create_title "Unleash The Fucking Fury" "cadaver-unleash"

# MARTIN GALWAY
file_author="Martin Galway"

./sid2sapr $stereo -b $basstype sid/'Ocean_Loader_1.sid'
create_title "Ocean Loader 1" "galway-ocean1"

./sid2sapr $stereo -b $basstype sid/'Ocean_Loader_2.sid'
create_title "Ocean Loader 2" "galway-ocean2"

./sid2sapr -a $stereo -b $basstype sid/'Terra_Cresta.sid'
create_title "Terra Cresta" "galway-terra"

# PETER CLARKE
file_author="Peter Clarke"

./sid2sapr $stereo -b $basstype sid/'Ocean_Loader_3.sid'
create_title "Ocean Loader 3" "clarke-ocean3"

# CHRIS HUELSBECK
file_author="Chris Huelsbeck & Ramiro Vaca"

./sid2sapr $stereo -b $basstype sid/'R-Type.sid'
create_title "R-Type" "huelsbeck-rtype"

# CHARLES DEENEN
file_author="Charles Deenen"

./sid2sapr $stereo -m both -f -b $basstype sid/'Zamzara.sid'
create_title "Zamzara" "deenen-zamzara"

# NTSC VBI 60HZ TEST

./sid2sapr $stereo -m all -b $basstype sid/'Snowflake.sid'
echo -n \
    "Source: Commodore 64                   " \
    "Title : Snowflake                      " \
    "Author: Anthony Butch Davis (Deathlok)" > asm/songname.txt
make compress$player player60$player
mv player.xex xex$player/deathlok-snowflake.xex

# Original of 100HZ test cover
file_author="Martin Galway"

./sid2sapr $stereo -t2 -m all -b $basstype sid/'Rastan.sid'
create_title "Rastan (tune 2)" "galway-rastan-tune2"

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
rm asm/songname.txt

# remove one that's too big
rm xex/hubbard-ik.xex
