#! /bin/sh

set -e

make

#if false; then

# BBC MICRO

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

# Sega Game 1000

./vgm2sapr sega/'H.E.R.O. (SG) - 02 - Main Theme.vgm'
echo -n \
    "Source: Sega Game 1000                 " \
    "Title : H.E.R.O Main Theme             " \
    "Author: " > asm/songname.txt
make compress
make player60
mv player.xex xex/sg1k-hero.xex

# Sega Master System

./vgm2sapr sega/'Sonic The Hedgehog - 03 - Green Hill Zone.vgm'
echo -n \
    "Source: Sega Game Master System        " \
    "Title : Sonic - Green Hill Zone        " \
    "Author: Masato Nakamura, Yuzo Koshiro" > asm/songname.txt
make compress
make player60
mv player.xex xex/sms-sonic-ghz.xex

./vgm2sapr sega/'Ace of Aces - 01 - Title Screen.vgm'
echo -n \
    "Source: Sega Game Master System        " \
    "Title : Ace Of Aces - Title Screen     " \
    "Author: " > asm/songname.txt
make compress
make player60
mv player.xex xex/sms-aceofaces.xex

./vgm2sapr sega/'system-of-masters.vgz'
echo -n \
    "Source: Sega Game Master System        " \
    "Title : System Of Masters              " \
    "Author: Alex Mauer" > asm/songname.txt
make compress
make player60
mv player.xex xex/sms-system.xex

# Sega Game Gear

./vgm2sapr sega/'The Incredible Hulk - 01 - Unused Title Theme.vgm'
echo -n \
    "Source: Sega Game Gear                 " \
    "Title : The Incredible Hulk Title Theme" \
    "Author: Steve Collett, Matt Furniss" > asm/songname.txt
make compress
make player60
mv player.xex xex/gg-hulk.xex

# no framerate in the file
./vgm2sapr -r 60 sega/'Sonic the Hedgehog 2 - 20 - Good Ending (Game Gear).vgm'
echo -n \
    "Source: Sega Game Gear                 " \
    "Title : Sonic 2 - Good Ending          " \
    "Author: Masafumi Ogata" > asm/songname.txt
make compress
make player60
mv player.xex xex/gg-sonic2.xex

./vgm2sapr sega/'Spider-Man - X-Men - Arcades Revenge - 01 - Title.vgm'
echo -n \
    "Source: Sega Game Gear                 " \
    "Title : Spider-Man / X-Men: AR         " \
    "Author: G.Follin, T.Follin, A.Brimble" > asm/songname.txt
make compress
make player60
mv player.xex xex/gg-spider.xex

# TANDY 1000

./vgm2sapr -r 60 tandy/'Zeliard - 02 Departure.vgz'
echo -n \
    "Source: Tandy 1000                     " \
    "Title : Zeliard - Departure            " \
    "Author: Masakuni Mitsuhashi, H. Godai" > asm/songname.txt
make compress
make player60
mv player.xex xex/tandy-zeliard.xex

./vgm2sapr -r 60 tandy/'Wibarm - 01 Title.vgz'
echo -n \
    "Source: Tandy 1000                     " \
    "Title : Wibarm - Title                 " \
    "Author: Toshiya Yamanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/tandy-wibarm.xex

# IBM PCjr

./vgm2sapr -r 60 ibmpcjr/'KK3 - 19 Finale.vgz'
echo -n \
    "Source: IBM PCjr                       " \
    "Title : King's Quest III - Finale      " \
    "Author: Margaret Lowe" > asm/songname.txt
make compress
make player60
mv player.xex xex/ibmpcjr-kk3.xex

./vgm2sapr -r 60 ibmpcjr/'Crossfire (floppy version).vgz'
echo -n \
    "Source: IBM PCjr                       " \
    "Title : Crossfire (Floppy Version)     " \
    "Author: Jay Sullivan" > asm/songname.txt
make compress
make player60
mv player.xex xex/ibmpcjr-crossfire.xex

# SEGA PICO

./vgm2sapr -r 60 sega/'Sonic Gameworld.vgz'
echo -n \
    "Source: SEGA Pico                      " \
    "Title : Sonic The Hedgehog's Gameworld " \
    "Author: Kojiro Mikusa" > asm/songname.txt
make compress
make player60
mv player.xex xex/pico-sonicgw.xex

# ARCADE

# completely different clock of 1536000Hz!
./vgm2sapr -r 60 arcade/'Mr. Goemon - 08 Stage 2 BGM 3 ~ BGM 4.vgz'
echo -n \
    "Source: Arcade                         " \
    "Title : Mr. Goemon - Stage 2+3 BGM     " \
    "Author: Konami Kukeiha Club" > asm/songname.txt
make compress
make player60
mv player.xex xex/arcade-mrgoemon.xex

# GAME BOY

./vgm2sapr -r 60 gb/'Ninja Gaiden Shadow - Stage 5.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Ninja Gaiden Shadow - Stage 5  " \
    "Author: Hiroyuki Iwatsuki" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-ninjagaiden.xex

./vgm2sapr -r 60 gb/'Super Mario Land - Staff Roll.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Super Mario Land - Staff Roll  " \
    "Author: Hirokazu Tanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-sml.xex

./vgm2sapr -r 60 gb/'Super Mario Land 2 - Athletic Theme.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Super Mario Land 2 - Athletic  " \
    "Author: Kazumi Totaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-sml2.xex

./vgm2sapr -r 60 gb/'Batman Animated Series - Batman Theme.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Batman Animated Series - Theme " \
    "Author: Yoshiyuki Hagiwara, D. Elfman" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-batman-theme.xex

./vgm2sapr -r 60 gb/'Batman - Title.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Batman - Title                 " \
    "Author: Nobuyuki Hara" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-batman-title.xex

./vgm2sapr -r 60 gb/'Blaster Master Boy - Ending.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Blaster Master Boy - Ending    " \
    "Author: Takeaki Kunimoto, Shinichi Seya" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-blaster-master.xex

./vgm2sapr -r 60 gb/'Mega Man IV - Title.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Mega Man IV - Title            " \
    "Author: Kouji Murata" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-megaman4-title.xex

./vgm2sapr -r 60 gb/'Spirou - Mountain.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Spirou - Mountain              " \
    "Author: Alberto Jose Gonzalez" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-spirou-mountain.xex

./vgm2sapr -r 60 gb/'Turok - Title Screen.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Turok - Title Screen           " \
    "Author: Alberto Jose Gonzalez" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-turok-title.xex

./vgm2sapr -r 60 gb/'Final Fantasy Adventure - Rising Sun.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : Final Fantasy Adv. - Rising Sun" \
    "Author: Kenji Ito" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-ffa-rising-sun.xex

./vgm2sapr -r 60 gb/'Final Fantasy Legend II - The Legend Begins.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : FF Legend 2 - The Legend Begins" \
    "Author: Nobuo Uematsu" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-ffl2-legend-begins.xex

./vgm2sapr -r 60 gb/'The Smurfs - Title Screen.vgz'
echo -n \
    "Source: Game Boy                       " \
    "Title : The Smurfs - Title Screen      " \
    "Author: Alberto Jose Gonzalez" > asm/songname.txt
make compress
make player60
mv player.xex xex/gb-ffl2-legend-begins.xex

# GAME BOY COLOR

./vgm2sapr -r 60 gbc/'Pokemon Card GB2 - GR Island.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Pokemon Card - GR Island       " \
    "Author: Ichiro Shimakura" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-pokemon-island.xex

./vgm2sapr -r 60 gbc/'Pokemon Card GB2 - Imakunis Theme.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Pokemon Card - Ikaminu's Theme " \
    "Author: Ichiro Shimakura" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-pokemon-ikamunis.xex

./vgm2sapr -r 60 gbc/'Pokemon Card GB2 - Staff Roll.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Pokemon Card - Staff Roll      " \
    "Author: Ichiro Shimakura" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-pokemon-roll.xex

./vgm2sapr -r 60 gbc/'Pokemon Card GB2 - Title Screen.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Pokemon Card - Title Screen    " \
    "Author: Ichiro Shimakura" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-pokemon-title.xex

./vgm2sapr -r 60 gbc/'Action Man - Moon Base 2.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Action Man - Moon Base 2       " \
    "Author: Iku Mizutani" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-actionman-moonbase2.xex

./vgm2sapr -r 60 gbc/'Beatmania - Hunting For You.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Beatmania - Hunting For You    " \
    "Author: Hiroyuki Togo" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-beatmania-hunting.xex

./vgm2sapr -r 60 gbc/'Pokemon Trading Card Game - Title.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Pokemon Trading Card Game Title" \
    "Author: Ichiro Shimakura" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-pokemon-trading.xex

./vgm2sapr -r 60 gbc/'Robocop - Robocop Theme.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Robocop - Robocop Theme        " \
    "Author: B.Poledouris, K.Wierzynkiewicz" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-robocop-theme.xex

./vgm2sapr -r 60 gbc/'Turok 2 - Base.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Turok 2 - Base                 " \
    "Author: Alberto Jose Gonzalez" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-turok2-base.xex

./vgm2sapr -r 60 gbc/'Lufia - The Legend Returns - Prologue.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Lufia Legend Returns - Prologue" \
    "Author: Yasunori Shiono, et al." > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-lufia-prologue.xex

./vgm2sapr -r 60 gbc/'Super Mario Bros. Deluxe - Running About.vgz'
echo -n \
    "Source: Game Boy Color                 " \
    "Title : Super Mario BD - Running About " \
    "Author: Koji Kondo" > asm/songname.txt
make compress
make player60
mv player.xex xex/gbc-smbd-running-about.xex

#fi

# clear for further tests
echo -n " " > asm/songname.txt
