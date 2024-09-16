#! /bin/sh

set -ex

make

./aylet -y -t 1 spectrum/BackToTheFuture2.ay > ../ym2sapr/spectrum/BackToTheFuture2.ym
./aylet -y -t 1 spectrum/BeverlyHillsCop.ay > ../ym2sapr/spectrum/BeverlyHillsCop.ym
./aylet -y -A 56 -F 1 spectrum/MissileGroundZero.AY > ../ym2sapr/spectrum/MissileGroundZero.ym

./aylet -y -c -t 1 cpc/'twistn shout 1.ay' > ../ym2sapr/cpc/'twistn shout 1.ym'
./aylet -y -c -t 1 cpc/"Burnin'Rubber.ay" > ../ym2sapr/cpc/"Burnin'Rubber..ym"

