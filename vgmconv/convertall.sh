#! /bin/sh

make

./vgmconv -r 60 -f -o ../ym2sapr/msx/dsiv-opening.ym \
          msx/'Dragon Slayer IV - 02 Opening Theme 2.vgz'

