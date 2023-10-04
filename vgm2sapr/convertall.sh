#! /bin/sh

set -e

make

if false; then

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
mv player.xex xex/gb-smurfs-title.xex

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

# TURBOGRAFX-16 / PC ENGINE

./vgm2sapr -m 16 -r 60 pcengine/'Bomberman 94 - Jammin Jungle.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Bomberman '94 - Jammin Jungle  " \
    "Author: Jun Chikuma, Hajime Kowara" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-bomberman94.xex

./vgm2sapr -r 60 pcengine/'Dragon'\''s Cruse - Take It Easy (Sunken Ship).vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Dragon's Curse - Take It Easy  " \
    "Author: Shinichi Sakamoto" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-dragons-curse.xex

./vgm2sapr -r 60 pcengine/'Magical Chase - Ending.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Magical Chase - Ending         " \
    "Author: Hitoshi Sakimoto" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-magical-chase-ending.xex

./vgm2sapr -r 60 pcengine/"Magical Chase - You're Restless.vgz"
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Magical Chase - You're Restless" \
    "Author: Masaharu Iwata" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-magical-chase-restless.xex

./vgm2sapr -r 60 pcengine/'Snatcher - Cold Sleep.vgz'
echo -n \
    "Source: TurboGrafx-16 PC Engine CD-ROM " \
    "Title : Snatcher - Cold Sleep          " \
    "Author: Konami Kukeiha Club" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-snatcher-cold-sleep.xex

./vgm2sapr -r 60 pcengine/'Soldier Blade - Opening.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Soldier Blade - Opening        " \
    "Author: Keita Hoshi, Makiko Tanifuji" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-soldier-blade-opening.xex

./vgm2sapr -r 60 pcengine/'The Legend Of Xanadu II - Underground Water Course.vgz'
echo -n \
    "Source: NEC PC Engine CD               " \
    "Title : Xanadu II - Underground Water C" \
    "Author: Atsushi Shirakawa, et al." > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-xanadu2-water-course.xex

./vgm2sapr -r 60 pcengine/'Ys IV: The Dawn Of Ys - On The Other Side Of The Recollection.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Ys IV: Other Side of the Recol." \
    "Author: Falcom Sound Team J.D.K." > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-ys-recollection.xex

./vgm2sapr -r 60 pcengine/'After Burner II - After Burner.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : After Burner II - After Burner " \
    "Author: Hiroshi Kawaguchi" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-after-burner2.xex

./vgm2sapr -r 60 pcengine/'Cratermaze - Booby Samba.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Cratermaze - Booby Samba       " \
    "Author: Kenji Yoshida, et al." > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-cratermaze-booby-samba.xex

./vgm2sapr -r 60 pcengine/'Terra Cresta II - Ending.vgz'
echo -n \
    "Source: TurboGrafx-16 / PC Engine      " \
    "Title : Terra Cresta II - Ending       " \
    "Author: Akihiro Akamatsu, Sachiko Oita" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-terra-cresta2-ending.xex

fi

./vgm2sapr -r 60 pcengine/'Tengai Makyou - Fuun Kabukiden'/'07 Great Merchant.vgz'
echo -n \
    "Source: PC Engine Super CD-ROM2        " \
    "Title : Tengai Makyou - Great Merchant " \
    "Author: Kohei Tanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-tmfk-great-merchant.xex

./vgm2sapr -r 60 pcengine/'Tengai Makyou - Fuun Kabukiden'/'13 Sign of the Demon King.vgz'
echo -n \
    "Source: PC Engine Super CD-ROM2        " \
    "Title : TM:FK - Sign Of The Demon King " \
    "Author: Kohei Tanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-tmfk-sign-demon-king.xex

./vgm2sapr -r 60 pcengine/'Tengai Makyou - Fuun Kabukiden'/'24 Secret Maneuvering.vgz'
echo -n \
    "Source: PC Engine Super CD-ROM2        " \
    "Title : Tengai Mak.  Secret Maneuvering" \
    "Author: Kohei Tanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-tmfk-secret-maneuvering.xex

./vgm2sapr -r 60 pcengine/'Tengai Makyou - Fuun Kabukiden'/'40 Fujiyama.vgz'
echo -n \
    "Source: PC Engine Super CD-ROM2        " \
    "Title : Tengai Makyou - Fujiyama       " \
    "Author: Kohei Tanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-tmfk-fujiyama.xex

./vgm2sapr -r 60 pcengine/'Tengai Makyou - Fuun Kabukiden'/'57 London Tower.vgz'
echo -n \
    "Source: PC Engine Super CD-ROM2        " \
    "Title : Tengai Makyou - London Tower   " \
    "Author: Kohei Tanaka" > asm/songname.txt
make compress
make player60
mv player.xex xex/pce-tmfk-london-tower.xex

# clear for further tests
echo -n " " > asm/songname.txt
