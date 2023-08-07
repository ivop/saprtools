#! /bin/sh

set -e

make

#if false; then

./ym2sapr -m -b softbass atarist/'seven gates of jambala 2 - intro.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Seven Gates Of Jambala Intro   " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/7gates-intro.xex

./ym2sapr -m -b softbass atarist/'seven gates of jambala 1 - level 11.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Seven Gates Of Jambala Level 11" \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/7gates-level11.xex

./ym2sapr -m -b softbass atarist/'enchanted land - game overa.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Game Over     " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-game-over.xex

# fix enveloped bass ;)
./ym2sapr -m -b softbass -e 13 -f 4 atarist/'enchanted land - introa.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Intro         " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-intro.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 1.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 1       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l1.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 2.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 2       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l2.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 3.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 3       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l3.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 4.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 4       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l4.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 5.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 5       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l5.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 6.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 6       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l6.xex

./ym2sapr -m -b softbass atarist/'enchanted land - level 7.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 7       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/enchanted-l7.xex

./ym2sapr -m -b softbass atarist/"ooh crickey wot a scorcher - there aren't any sheep in outer mongolia.ym"
echo -n \
    "Source: Atari ST        (There Aren't) " \
    "Title : Any Sheep In Outer Mongolia    " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/crickey-sheep.xex

./ym2sapr -m -b softbass atarist/'ooh crickey wot a scorcher - things that go bump.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Things That Go Bump            " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/crickey-things.xex

./ym2sapr -m -b softbass atarist/'ooh crickey wot a scorcher - your mind is my ashtray.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Your Mind Is My Ashtray        " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/crickey-ashtray.xex

# fix high speed envelope notes ;)
./ym2sapr -m -b softbass -e 13 -f 32 atarist/'decade demo - gigadist-a.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Decade Demo - Gigadist         " \
    "Author: Count Zero" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/decade-gigadist.xex

./ym2sapr -m -b softbass atarist/'weird dreams - 1.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Weird Dreams 1                 " \
    "Author: David Whittaker" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/whittaker-weird-dreams.xex

./ym2sapr -p 9 -m -b softbass spectrum/BackToTheFuture2.ym
echo -n \
    "Source: ZX Spectrum                    " \
    "Title : Back To The Future 2           " \
    "Author: David Whittaker" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/whittaker-bttf2.xex

./ym2sapr -m -b softbass spectrum/BeverlyHillsCop.ym
echo -n \
    "Source: ZX Spectrum                    " \
    "Title : Beverly Hills Cop              " \
    "Author: David Whittaker" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/whittaker-axelf.xex

# drums and noise on the same side
./ym2sapr -m -b softbass -r acb cpc/"Burnin'Rubber..ym"
echo -n \
    "Source: Amstrad CPC                    " \
    "Title : Burnin'Rubber                  " \
    "Author: Jonathan Dunn & Matthew Cannon" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/burnin.xex

# drums and noise on the same side
./ym2sapr -m -b softbass -r acb cpc/'twistn shout 1.ym'
echo -n \
    "Source: Amstrad CPC                    " \
    "Title : Twist'n'shout part, Voyage 93  " \
    "Author: Kangaroo" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/twist.xex

./ym2sapr -m -b softbass spectrum/MissileGroundZero.ym
echo -n \
    "Source: ZX Spectrum                    " \
    "Title : Missile: Ground Zero           " \
    "Author: Tim Follin" > asm/songname.txt
make compress-softbass
make player-softbass
mv player.xex xex-softbass/missile-ground0.xex

./ym2sapr -m -b softbass msx/dsiv-opening.ym
echo -n \
    "Source: MSX/MSX2                       " \
    "Title : Dragon Slayer IV: Opening Theme" \
    "Author: Yuzo Koshiro, Mieko Ishikawa" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/dsiv-opening.xex

./ym2sapr -m -b softbass msx/dsiv-shop.ym
echo -n \
    "Source: MSX/MSX2                       " \
    "Title : Dragon Slayer IV: Theme of Shop" \
    "Author: Yuzo Koshiro, Mieko Ishikawa" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/dsiv-shop.xex

./ym2sapr -m -b softbass msx/dsiv-lyll.ym
echo -n \
    "Source: MSX/MSX2                       " \
    "Title : Dragon Slayer IV: Lyll Worzen  " \
    "Author: Yuzo Koshiro, Mieko Ishikawa" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/dsiv-lyll.xex

./ym2sapr -m -b softbass msx/dsiv-maia.ym
echo -n \
    "Source: MSX/MSX2                       " \
    "Title : Dragon Slayer IV: Maia Worzen  " \
    "Author: Yuzo Koshiro, Mieko Ishikawa" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/dsiv-maia.xex

./ym2sapr -m -b softbass msx/dsiv-ending.ym
echo -n \
    "Source: MSX/MSX2                       " \
    "Title : Dragon Slayer IV: Ending Theme " \
    "Author: Yuzo Koshiro, Mieko Ishikawa" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/dsiv-ending.xex

./ym2sapr -m -b softbass msx/xak-path.ym
echo -n \
    "Source: MSX2                           " \
    "Title : Xak: TAoVS: Path To The Fort   " \
    "Author: Tadahiro Nitta, Ryuji Sasai" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/xak-path.xex

./ym2sapr -m -b softbass msx/xak-dragon.ym
echo -n \
    "Source: MSX2                           " \
    "Title : Xak: TAoVS: Water Dragon       " \
    "Author: Tadahiro Nitta, Ryuji Sasai" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/xak-dragon.xex

./ym2sapr -m -b softbass msx/xak-fort.ym
echo -n \
    "Source: MSX2                           " \
    "Title : Xak: TAoVS: Fort Of Flames     " \
    "Author: Tadahiro Nitta, Ryuji Sasai" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/xak-fort.xex

./ym2sapr -m -b softbass msx/xak-ending2.ym
echo -n \
    "Source: MSX2                           " \
    "Title : Xak: TAoVS: Ending 2           " \
    "Author: Tadahiro Nitta, Ryuji Sasai" > asm/songname.txt
make compress-softbass
make player60-softbass
mv player.xex xex-softbass/xak-ending2.xex

#fi

# clear for further tests
echo -n " " > asm/songname.txt
