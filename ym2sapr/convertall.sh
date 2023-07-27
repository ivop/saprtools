#! /bin/sh

set -e

make

./ym2sapr atarist/'seven gates of jambala 2 - intro.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Seven Gates Of Jambala Intro   " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/7gates-intro.xex

./ym2sapr atarist/'seven gates of jambala 1 - level 11.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Seven Gates Of Jambala Level 11" \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/7gates-level11.xex

./ym2sapr atarist/'enchanted land - game overa.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Game Over     " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-game-over.xex

# fix enveloped bass ;)
./ym2sapr -e 13 -f 4 atarist/'enchanted land - introa.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Intro         " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-intro.xex

./ym2sapr atarist/'enchanted land - level 1.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 1       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l1.xex

./ym2sapr atarist/'enchanted land - level 2.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 2       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l2.xex

./ym2sapr atarist/'enchanted land - level 3.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 3       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l3.xex

./ym2sapr atarist/'enchanted land - level 4.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 4       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l4.xex

./ym2sapr atarist/'enchanted land - level 5.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 5       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l5.xex

./ym2sapr atarist/'enchanted land - level 6.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 6       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l6.xex

./ym2sapr atarist/'enchanted land - level 7.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Enchanted Land - Level 7       " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/enchanted-l7.xex

./ym2sapr atarist/"ooh crickey wot a scorcher - there aren't any sheep in outer mongolia.ym"
echo -n \
    "Source: Atari ST        (There Aren't) " \
    "Title : Any Sheep In Outer Mongolia    " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/crickey-sheep.xex

./ym2sapr atarist/'ooh crickey wot a scorcher - things that go bump.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Things That Go Bump            " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/crickey-things.xex

./ym2sapr atarist/'ooh crickey wot a scorcher - your mind is my ashtray.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Your Mind Is My Ashtray        " \
    "Author: Jochen Hippel" > asm/songname.txt
make compress
make player
mv player.xex xex/crickey-ashtray.xex

# fix high speed envelope notes ;)
./ym2sapr -e 13 -f 32 atarist/'decade demo - gigadist-a.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Decade Demo - Gigadist         " \
    "Author: Count Zero" > asm/songname.txt
make compress
make player
mv player.xex xex/decade-gigadist.xex

./ym2sapr atarist/'weird dreams - 1.ym'
echo -n \
    "Source: Atari ST                       " \
    "Title : Weird Dreams 1                 " \
    "Author: David Whittaker" > asm/songname.txt
make compress
make player
mv player.xex xex/whittaker-weird-dreams.xex

# clear for further tests
echo -n " " > asm/songname.txt
