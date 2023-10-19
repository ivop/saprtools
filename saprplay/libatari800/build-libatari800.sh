#! /bin/bash

cd libatari800
./autogen.sh
CFLAGS=-O3 ./configure --disable-sdltest --disable-cursesbasic --disable-crashmenu --disable-eventrecording --disable-audiorecording --disable-videorecording --disable-screenshots --disable-pngcodec --disable-zmbvcodec  --disable-opengl-by-default --disable-bufferdlog --disable-newcycleexact --target=libatari800
make -j8

