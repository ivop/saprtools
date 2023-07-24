
CXX ?= g++

ym2sapr: main.cpp lzh.cpp lzh.h
	$(CXX) -O2 -o ym2sapr main.cpp lzh.cpp

compress:
	lzss -6 left.sapr left.lz16
	lzss -6 right.sapr right.lz16

player:
	mads -o:player.xex asm/playlzs16-stereo.asm

clean:
	rm -f ym2sapr *~ */*~

cleaner: clean
	rm -f *.sapr *.lz16 player.xex

