#! /bin/sh
set -ex

cd ay2ym
./convertall.sh
cd ..

cd sid2sapr
./convertall.sh
cd ..

cd vgm2sapr
./convertall.sh
cd ..

cd ym2sapr
./convertall.sh
cd ..
