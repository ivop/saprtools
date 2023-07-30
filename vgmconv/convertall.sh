#! /bin/sh

make

./vgmconv -r 60 -f -o ../ym2sapr/msx/dsiv-opening.ym \
          msx/'Dragon Slayer IV - 02 Opening Theme 2.vgz'

./vgmconv -r 60 -f -o ../ym2sapr/msx/dsiv-shop.ym \
          msx/'Dragon Slayer IV - 07 Theme of Shop.vgz'

./vgmconv -r 60 -f -o ../ym2sapr/msx/dsiv-lyll.ym \
          msx/'Dragon Slayer IV - 12 Lyll Worzen.vgz'

./vgmconv -r 60 -f -o ../ym2sapr/msx/dsiv-maia.ym \
          msx/'Dragon Slayer IV - 14 Maia Worzen.vgz'

./vgmconv -r 60 -f -o ../ym2sapr/msx/dsiv-ending.ym \
          msx/'Dragon Slayer IV - 17 Ending Theme.vgz'
