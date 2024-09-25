CXX ?= g++

LZSS = ../lzss-sap/bin/lzss

all: ym2sapr

convertall: ym2sapr
	./convertall.sh

ym2sapr: main.cpp lzh.cpp lzh.h
	$(CXX) $(STATIC) -O2 -o ym2sapr main.cpp lzh.cpp -lm

compress-stereo: $(LZSS)
	@$(LZSS) -6 left.sapr left.lz16   >>convertall.log  2>&1
	@$(LZSS) -6 right.sapr right.lz16 >>convertall.log  2>&1

compress-mono: $(LZSS)
	@$(LZSS) -6 left.sapr left.lz16  >>convertall.log 2>&1

compress-softbass: compress-mono

player50-stereo:
	mads -o:player.xex ../player/asm/playlzs16-stereo.asm

player50-mono:
	mads -o:player.xex ../player/asm/playlzs16-mono.asm

player50-softbass:
	mads -o:player.xex ../player/asm/playlzs16-softbass.asm

player60-stereo:
	mads -d:HZ60 -o:player.xex ../player/asm/playlzs16-stereo.asm


player60-mono:
	mads -d:HZ60 -o:player.xex ../player/asm/playlzs16-mono.asm

player60-softbass:
	mads -d:HZ60 -o:player.xex ../player/asm/playlzs16-softbass.asm

$(LZSS):
	+make -C ../lzss-sap

clean:
	rm -f *.exe *.log *~ */*~ ym2sapr

cleaner: clean
	rm -f *.sapr *.lz16 player.xex
