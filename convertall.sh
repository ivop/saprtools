#! /bin/sh
pushd aylet
./convertall.sh
popd

pushd sid2sapr
./convertall.sh
popd

pushd vgm2sapr
./convertall.sh
popd

pushd vgm2ym
./convertall.sh
popd

pushd ym2sapr
./convertall.sh
./convertall-mono.sh
./convertall-softbass.sh
popd
