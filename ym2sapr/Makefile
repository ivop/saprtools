
CXX ?= g++

LZSS = ../lzss-sap/bin/lzss

ym2sapr: main.cpp lzh.cpp lzh.h
	$(CXX) -O2 -o ym2sapr main.cpp lzh.cpp -lm

compress: $(LZSS)
	$(LZSS) -6 left.sapr left.lz16
	$(LZSS) -6 right.sapr right.lz16

compress-mono: $(LZSS)
	$(LZSS) -6 left.sapr left.lz16

compress-softbass: compress-mono

player:
	mads -o:player.xex asm/playlzs16-stereo.asm

player60:
	mads -d:HZ60 -o:player.xex asm/playlzs16-stereo.asm

player-mono:
	mads -o:player.xex asm/playlzs16-mono.asm

player60-mono:
	mads -d:HZ60 -o:player.xex asm/playlzs16-mono.asm

player-softbass:
	mads -o:player.xex asm/playlzs16-softbass.asm

player60-softbass:
	mads -d:HZ60 -o:player.xex asm/playlzs16-softbass.asm

$(LZSS):
	make -C ../lzss-sap

clean:
	rm -f ym2sapr *~ */*~

cleaner: clean
	rm -f *.sapr *.lz16 player.xex

