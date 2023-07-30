#! /bin/sh

make

./vgmconv -o ../ym2sapr/msx/dsiv-opening.ym \
          -r 59.94 \
          msx/'Dragon Slayer IV - 02 Opening Theme 2.vgz'

