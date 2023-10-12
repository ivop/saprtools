#! /bin/sh

./sid2sapr -p 12 -x 1 -e 1 -E 1.0 -b softbass sid/Syncopated.sid
make compress-softbass player-softbass-sawtooth
mv player.xex experimental/exp-syncopated.xex

./sid2sapr -p 15 -x 2 -e 2 -E 0.6 -b softbass sid/Metal_Warrior_4.sid
make compress-softbass player-softbass-sawtooth
mv player.xex experimental/exp-mw4.xex

./sid2sapr -m both -p 10 -x 2 -e 2 -E 0.8 -b softbass sid/Zamzara.sid
make compress-softbass player-softbass-sawtooth
mv player.xex experimental/exp-zamzara.xex

