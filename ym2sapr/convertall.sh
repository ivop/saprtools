#! /bin/sh

set -e

. ../player/convert.sh

create_sapr() {
  ./ym2sapr ${basstype} "$@" >>convertall.log 2>&1
}

echo >convertall.log

file_converter="ym2sapr"

for player in "stereo" "mono" "softbass" ; do
echo "Starting ${file_converter} conversion for ${player} player."

case "$player" in
  "stereo") basstype="" ;;
  "mono") basstype="-m -b buzzy" ;;
  "softbass") basstype="-m -b softbass" ;;
esac

#if false; then

# "Atari ST
file_source="Atari ST"

file_author="Jochen Hippel"

create_sapr "atarist/seven gates of jambala 2 - intro.ym"
create_title "Seven Gates Of Jambala Intro" "7gates-intro"

create_sapr "atarist/seven gates of jambala 1 - level 11.ym"
create_title "Seven Gates Of Jambala Level 11" "7gates-level11"

create_sapr "atarist/enchanted land - game overa.ym"
create_title "Enchanted Land - Game Over" "enchanted-game-over"

# fix enveloped bass ;)
create_sapr -e 13 -f 4 "atarist/enchanted land - introa.ym"
create_title "Enchanted Land - Intro" "enchanted-intro"

create_sapr "atarist/enchanted land - level 1.ym"
create_title "Enchanted Land - Level 1" "enchanted-l1"

create_sapr "atarist/enchanted land - level 2.ym"
create_title "Enchanted Land - Level 2" "enchanted-l2"

create_sapr "atarist/enchanted land - level 3.ym"
create_title "Enchanted Land - Level 3" "enchanted-l3"

create_sapr "atarist/enchanted land - level 4.ym"
create_title "Enchanted Land - Level 4" "enchanted-l4"

create_sapr "atarist/enchanted land - level 5.ym"
create_title "Enchanted Land - Level 5" "enchanted-l5"

create_sapr "atarist/enchanted land - level 6.ym"
create_title "Enchanted Land - Level 6" "enchanted-l6"

create_sapr "atarist/enchanted land - level 7.ym"
create_title "Enchanted Land - Level 7" "enchanted-l7"

# TODO title should not be part of source
file_source="Atari ST        (There Aren't)"
create_sapr "atarist/ooh crickey wot a scorcher - there aren't any sheep in outer mongolia.ym"
create_title "Any Sheep In Outer Mongolia" "crickey-sheep"
file_source="Atari ST"

create_sapr "atarist/ooh crickey wot a scorcher - things that go bump.ym"
create_title "Things That Go Bump" "crickey-things"

create_sapr "atarist/ooh crickey wot a scorcher - your mind is my ashtray.ym"
create_title "Your Mind Is My Ashtray" "crickey-ashtray"

create_sapr "../sndh2ym/ym/Car_Race.ym"
create_title "Car Race" "hippel-car-race"

file_author="Count Zero"
# fix high speed envelope notes ;)
create_sapr -e 13 -f 32 "atarist/decade demo - gigadist-a.ym"
create_title "Decade Demo - Gigadist" "decade-gigadist"

file_author="David Whittaker"
create_sapr "atarist/weird dreams - 1.ym"
create_title "Weird Dreams 1" "whittaker-weird-dreams"

# ZX Spectrum
file_source="ZX Spectrum"

file_author="David Whittaker"
if [ "${player}" = "softbass" ]; then
  create_sapr -p 9 "spectrum/BackToTheFuture2.ym"
else
  create_sapr "spectrum/BackToTheFuture2.ym"
fi
create_title "Back To The Future 2" "whittaker-bttf2"

create_sapr "spectrum/BeverlyHillsCop.ym"
create_title "Beverly Hills Cop" "whittaker-axelf"

file_author="nq / Oleg Nikitin"
create_sapr -e 13 -f 8 "../psg2ym/ym/2018_nq_skrju_demosong.ym"
create_title "Vortextracker Demosong" "vortex-demosong"

file_author="MmcM / Sergey Kosov"
create_sapr -e 12 -f 8 "../psg2ym/ym/2019_MmcM_Conversions.ym"
create_title "Conversions" "vortex-conversions"

create_sapr "../psg2ym/ym/MmcM - Fragment-2024 Realtime compose (2024) (DiHalt 2024 Winter, 3).ym"
create_title "Fragment 2024 Realtime Compose" "mmcm-fragment24"

create_sapr "../psg2ym/ym/MmcM - The Bright Rays Of You (2024) (Multimatograf 2024, 1).ym"
create_title "The Bright Rays Of You" "mmcm-brightrays"

#Amstrad CPC
file_source="Amstrad CPC"

file_author="Jonathan Dunn & Matthew Cannon"
# drums and noise on the same side
create_sapr -r acb "cpc/Burnin'Rubber.ym"
create_title "Burnin'Rubber" "burnin"

file_author="Kangaroo"
# drums and noise on the same side
create_sapr -r acb "cpc/twistn shout 1.ym"
create_title "Twist'n'shout part, Voyage 93" "twist"

# ZX Spectrum
file_source="ZX Spectrum"

file_author="Tim Follin"
create_sapr "spectrum/MissileGroundZero.ym"
create_title "Missile: Ground Zero" "missile-ground0"

# MSX/MSX2
file_source="MSX/MSX2"

file_author="Yuzo Koshiro, Mieko Ishikawa"
create_sapr "msx/dsiv-opening.ym"
create_title "Dragon Slayer IV: Opening Theme" "dsiv-opening" "60"

create_sapr "msx/dsiv-shop.ym"
create_title "Dragon Slayer IV: Theme of Shop" "dsiv-shop" "60"

create_sapr "msx/dsiv-lyll.ym"
create_title "Dragon Slayer IV: Lyll Worzen" "dsiv-lyll" "60"

create_sapr "msx/dsiv-maia.ym"
create_title "Dragon Slayer IV: Maia Worzen" "dsiv-maia" "60"

create_sapr "msx/dsiv-ending.ym"
create_title "Dragon Slayer IV: Ending Theme" "dsiv-ending" "60"

# MSX2
file_source="MSX2"

file_author="Tadahiro Nitta, Ryuji Sasai"
create_sapr "msx/xak-path.ym"
create_title "Xak: TAoVS: Path To The Fort" "xak-path" "60"

create_sapr "msx/xak-dragon.ym"
create_title "Xak: TAoVS: Water Dragon" "xak-dragon" "60"

create_sapr "msx/xak-fort.ym"
create_title "Xak: TAoVS: Fort Of Flames" "xak-fort" "60"

create_sapr "msx/xak-ending2.ym"
create_title "Xak: TAoVS: Ending 2" "xak-ending2" "60"

#fi

done

# clear for further tests
rm songname.txt
