#! /bin/sh

set -ex

make

./ay2ym -A 194 -t 1 -o ../ym2sapr/spectrum/BackToTheFuture2.ym spectrum/BackToTheFuture2.ay

./ay2ym -A 194 -t 1 -o../ym2sapr/spectrum/BeverlyHillsCop.ym spectrum/BeverlyHillsCop.ay

./ay2ym -A 61 -o ../ym2sapr/spectrum/MissileGroundZero.ym spectrum/MissileGroundZero.AY

./ay2ym -A 194 -c -t 1 -o ../ym2sapr/cpc/'twistn shout 1.ym' cpc/'twistn shout 1.ay'
./ay2ym -A 194 -c -t 1 -o ../ym2sapr/cpc/"Burnin'Rubber.ym" cpc/"Burnin'Rubber.ay"

