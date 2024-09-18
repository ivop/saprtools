#! /bin/sh

set -e

. ../player/convert.sh

create_sapr() {
  ./vgm2sapr "$@" >>convertall.log 2>&1
}

make
echo >convertall.log

file_converter="ym2sapr"

for player in "stereo" ; do
echo "Starting ${file_converter} conversion for ${player} player."

#if false; then

# BBC MICRO
file_source="BBC Micro"

file_author="Martin Galway"

create_sapr "bbc/CrazeeRider-title.vgz"
create_title "Crazee Rider" "bbc-crazeerider" "50"

create_sapr "bbc/Galaforce2-title.vgz"
create_title "Galaforce 2" "bbc-galaforce2" "50"

file_author="Nick Pelling"
create_sapr bbc/Firetrack-ingame.vgz
create_title "Firetrack" "bbc-firetrack" "50"

file_author="Nigel Scott"
create_sapr "bbc/TubularBells.vgz"
create_title "Tubular Bells (demo)" "bbc-tubular" "50"

# Sega Game 1000
file_source="Sega Game 1000"

file_author=""
create_sapr "sega/H.E.R.O. (SG) - 02 - Main Theme.vgm"
create_title "H.E.R.O Main Theme" "sg1k-hero" "60"

# Sega Master System
file_source="Sega Game Master System"

file_author="Masato Nakamura, Yuzo Koshiro"
create_sapr "sega/Sonic The Hedgehog - 03 - Green Hill Zone.vgm"
create_title "Sonic - Green Hill Zone" "sms-sonic-ghz" "60"

file_author=""
create_sapr "sega/Ace of Aces - 01 - Title Screen.vgm"
create_title "Ace Of Aces - Title Screen" "sms-aceofaces" "60"

file_author="Alex Mauer"
create_sapr "sega/system-of-masters.vgz"
create_title "System Of Masters" "sms-system" "60"

# Sega Game Gear
file_source="Sega Game Gear"

file_author="Steve Collett, Matt Furniss"
create_sapr "sega/The Incredible Hulk - 01 - Unused Title Theme.vgm"
create_title "The Incredible Hulk Title Theme" "gg-hulk" "60"

file_author="Masafumi Ogata"
# no framerate in the file
create_sapr -r 60 "sega/Sonic the Hedgehog 2 - 20 - Good Ending (Game Gear).vgm"
create_title "Sonic 2 - Good Ending" "gg-sonic2" "60"

file_author="G.Follin, T.Follin, A.Brimble"
create_sapr "sega/Spider-Man - X-Men - Arcades Revenge - 01 - Title.vgm"
create_title "Spider-Man / X-Men: AR" "gg-spider" "60"

# TANDY 1000
file_source="Tandy 1000"

file_author="Masakuni Mitsuhashi, H. Godai"
create_sapr -r 60 "tandy/Zeliard - 02 Departure.vgz"
create_title "Zeliard - Departure" "tandy-zeliard" "60"

file_author="Toshiya Yamanaka"
create_sapr -r 60 "tandy/Wibarm - 01 Title.vgz"
create_title "Wibarm - Title" "tandy-wibarm" "60"

# IBM PCjr
file_source="IBM PCjr"

file_author="Margaret Lowe"
create_sapr -r 60 "ibmpcjr/KK3 - 19 Finale.vgz"
create_title "King's Quest III - Finale" "ibmpcjr-kk3" "60"

file_author="Jay Sullivan"
create_sapr -r 60 "ibmpcjr/Crossfire (floppy version).vgz"
create_title "Crossfire (Floppy Version)" "ibmpcjr-crossfire" "60"

# SEGA PICO
file_source="SEGA Pico"

file_author="Kojiro Mikusa"
create_sapr -r 60 "sega/Sonic Gameworld.vgz"
create_title "Sonic The Hedgehog's Gameworld" "pico-sonicgw" "60"

# ARCADE
file_source="Arcade"

file_author="Konami Kukeiha Club"
# completely different clock of 1536000Hz!
create_sapr -r 60 "arcade/Mr. Goemon - 08 Stage 2 BGM 3 ~ BGM 4.vgz"
create_title "Mr. Goemon - Stage 2+3 BGM" "arcade-mrgoemon" "60"

# GAME BOY
file_source="Game Boy"

file_author="Hiroyuki Iwatsuki"
create_sapr -r 60 "gb/Ninja Gaiden Shadow - Stage 5.vgz"
create_title "Ninja Gaiden Shadow - Stage 5" "gb-ninjagaiden" "60"

file_author="Hirokazu Tanaka"
create_sapr -r 60 "gb/Super Mario Land - Staff Roll.vgz"
create_title "Super Mario Land - Staff Roll" "gb-sml" "60"

file_author="Kazumi Totaka"
create_sapr -r 60 "gb/Super Mario Land 2 - Athletic Theme.vgz"
create_title "Super Mario Land 2 - Athletic" "gb-sml2" "60"

file_author="Yoshiyuki Hagiwara, D. Elfman"
create_sapr -r 60 "gb/Batman Animated Series - Batman Theme.vgz"
create_title "Batman Animated Series - Theme" "gb-batman-theme" "60"

file_author="Nobuyuki Hara"
create_sapr -r 60 "gb/Batman - Title.vgz"
create_title "Batman - Title" "gb-batman-title" "60"

file_author="Takeaki Kunimoto, Shinichi Seya"
create_sapr -r 60 "gb/Blaster Master Boy - Ending.vgz"
create_title "Blaster Master Boy - Ending" "gb-blaster-master" "60"

file_author="Kouji Murata"
create_sapr -r 60 "gb/Mega Man IV - Title.vgz"
create_title "Mega Man IV - Title" "gb-megaman4-title" "60"

file_author="Alberto Jose Gonzalez"
create_sapr -r 60 "gb/Spirou - Mountain.vgz"
create_title "Spirou - Mountain" "gb-spirou-mountain" "60"

file_author="Alberto Jose Gonzalez"
create_sapr -r 60 "gb/Turok - Title Screen.vgz"
create_title "Turok - Title Screen" "gb-turok-title" "60"

file_author="Kenji Ito"
create_sapr -r 60 "gb/Final Fantasy Adventure - Rising Sun.vgz"
create_title "Final Fantasy Adv. - Rising Sun" "gb-ffa-rising-sun" "60"

file_author="Nobuo Uematsu"
create_sapr -r 60 "gb/Final Fantasy Legend II - The Legend Begins.vgz"
create_title "FF Legend 2 - The Legend Begins" "gb-ffl2-legend-begins" "60"

file_author="Alberto Jose Gonzalez"
create_sapr -r 60 "gb/The Smurfs - Title Screen.vgz"
create_title "The Smurfs - Title Screen" "gb-smurfs-title" "60"

# GAME BOY COLOR
file_source="Game Boy Color"

file_author="Ichiro Shimakura"
create_sapr -r 60 "gbc/Pokemon Card GB2 - GR Island.vgz"
create_title "Pokemon Card - GR Island" "gbc-pokemon-island" "60"

file_author="Ichiro Shimakura"
create_sapr -r 60 "gbc/Pokemon Card GB2 - Imakunis Theme.vgz"
create_title "Pokemon Card - Ikaminu's Theme" "gbc-pokemon-ikamunis" "60"

create_sapr -r 60 "gbc/Pokemon Card GB2 - Staff Roll.vgz"
create_title "Pokemon Card - Staff Roll" "gbc-pokemon-roll" "60"

create_sapr -r 60 "gbc/Pokemon Card GB2 - Title Screen.vgz"
create_title "Pokemon Card - Title Screen" "gbc-pokemon-title" "60"

file_author="Iku Mizutani"
create_sapr -r 60 "gbc/Action Man - Moon Base 2.vgz"
create_title "Action Man - Moon Base 2" "gbc-actionman-moonbase2" "60"

file_author="Hiroyuki Togo"
create_sapr -r 60 "gbc/Beatmania - Hunting For You.vgz"
create_title "Beatmania - Hunting For You" "gbc-beatmania-hunting" "60"

file_author="Ichiro Shimakura"
create_sapr -r 60 "gbc/Pokemon Trading Card Game - Title.vgz"
create_title "Pokemon Trading Card Game Title" "gbc-pokemon-trading" "60"

file_author="B.Poledouris, K.Wierzynkiewicz"
create_sapr -r 60 "gbc/Robocop - Robocop Theme.vgz"
create_title "Robocop - Robocop Theme" "gbc-robocop-theme" "60"

file_author="Alberto Jose Gonzalez"
create_sapr -r 60 "gbc/Turok 2 - Base.vgz"
create_title "Turok 2 - Base" "gbc-turok2-base" "60"

file_author="Yasunori Shiono, et al."
create_sapr -r 60 "gbc/Lufia - The Legend Returns - Prologue.vgz"
create_title "Lufia Legend Returns - Prologue" "gbc-lufia-prologue" "60"

file_author="Koji Kondo"
create_sapr -r 60 "gbc/Super Mario Bros. Deluxe - Running About.vgz"
create_title "Super Mario BD - Running About" "gbc-smbd-running-about" "60"

# TURBOGRAFX-16 / PC ENGINE
file_source="TurboGrafx-16 / PC Engine"

file_author="Jun Chikuma, Hajime Kowara"
create_sapr -m 16 -r 60 "pcengine/Bomberman 94 - Jammin Jungle.vgz"
create_title "Bomberman '94 - Jammin Jungle" "pce-bomberman94" "60"

file_author="Shinichi Sakamoto"
create_sapr -r 60 "pcengine/Dragon's Cruse - Take It Easy (Sunken Ship).vgz"
create_title "Dragon's Curse - Take It Easy" "pce-dragons-curse" "60"

file_author="Hitoshi Sakimoto"
create_sapr -r 60 "pcengine/Magical Chase - Ending.vgz"
create_title "Magical Chase - Ending" "pce-magical-chase-ending" "60"

file_author="Masaharu Iwata"
create_sapr -r 60 pcengine/"Magical Chase - You're Restless.vgz"
create_title "Magical Chase - You're Restless" "pce-magical-chase-restless" "60"

file_author="Keita Hoshi, Makiko Tanifuji"
create_sapr -r 60 "pcengine/Soldier Blade - Opening.vgz"
create_title "Soldier Blade - Opening" "pce-soldier-blade-opening" "60"

file_author="Falcom Sound Team J.D.K."
create_sapr -r 60 "pcengine/Ys IV The Dawn Of Ys - On The Other Side Of The Recollection.vgz"
create_title "Ys IV: Other Side of the Recol." "pce-ys-recollection" "60"

file_author="Hiroshi Kawaguchi"
create_sapr -r 60 "pcengine/After Burner II - After Burner.vgz"
create_title "After Burner II - After Burner" "pce-after-burner2" "60"

file_author="Kenji Yoshida, et al."
create_sapr -r 60 "pcengine/Cratermaze - Booby Samba.vgz"
create_title "Cratermaze - Booby Samba" "pce-cratermaze-booby-samba" "60"

file_author="Akihiro Akamatsu, Sachiko Oita"
create_sapr -r 60 "pcengine/Terra Cresta II - Ending.vgz"
create_title "Terra Cresta II - Ending" "pce-terra-cresta2-ending" "60"

file_source="TurboGrafx-16 PC Engine CD-ROM"
file_author="Konami Kukeiha Club"
create_sapr -r 60 "pcengine/Snatcher - Cold Sleep.vgz"
create_title "Snatcher - Cold Sleep" "pce-snatcher-cold-sleep" "60"

file_source="NEC PC Engine CD"
file_author="Atsushi Shirakawa, et al."
create_sapr -r 60 "pcengine/The Legend Of Xanadu II - Underground Water Course.vgz"
create_title "Xanadu II - Underground Water C" "pce-xanadu2-water-course" "60"

file_source="PC Engine Super CD-ROM2"

file_author="Kohei Tanaka"

create_sapr -r 60 "pcengine/Tengai Makyou - Fuun Kabukiden/07 Great Merchant.vgz"
create_title "Tengai Makyou - Great Merchant" "pce-tmfk-great-merchant" "60"

create_sapr -r 60 "pcengine/Tengai Makyou - Fuun Kabukiden/13 Sign of the Demon King.vgz"
create_title "TM:FK - Sign Of The Demon King" "pce-tmfk-sign-demon-king" "60"

create_sapr -r 60 "pcengine/Tengai Makyou - Fuun Kabukiden/24 Secret Maneuvering.vgz"
create_title "Tengai Mak.  Secret Maneuvering" "pce-tmfk-secret-maneuvering" "60"

create_sapr -r 60 "pcengine/Tengai Makyou - Fuun Kabukiden/40 Fujiyama.vgz"
create_title "Tengai Makyou - Fujiyama" "pce-tmfk-fujiyama" "60"

create_sapr -r 60 "pcengine/Tengai Makyou - Fuun Kabukiden/57 London Tower.vgz"
create_title "Tengai Makyou - London Tower" "pce-tmfk-london-tower" "60"

#fi

done

# clear for further tests
rm songname.txt
