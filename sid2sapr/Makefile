CC ?= gcc
CFLAGS ?= -O2 -Wall -std=c99 -D_XOPEN_SOURCE=500
LIBS = -lm
LZSS = ../lzss-sap/bin/lzss

all: sid2sapr

convertall: sid2sapr
	./convertall.sh

sid2sapr: main.c c64.c m6502.h c64.h md5.c md5.h saw.h
		$(CC) $(STATIC) $(CFLAGS) -o sid2sapr main.c c64.c md5.c $(LIBS)

compress-stereo: $(LZSS)
	@$(LZSS) -6 left.sapr left.lz16   >>convertall.log  2>&1
	@$(LZSS) -6 right.sapr right.lz16 >>convertall.log  2>&1

compress-mono: $(LZSS)
	@$(LZSS) -6 output.sapr output.lz16 >>convertall.log 2>&1

compress-softbass: compress-mono

player50-stereo:
	mads -d:SID2SAPR $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-stereo.asm

player50-mono:
	mads -d:SID2SAPR $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-mono.asm

player50-softbass:
	mads -d:SID2SAPR $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-softbass.asm

player50-softbass-sawtooth:
	mads -d:SAWTOOTH -d:SID2SAPR $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-softbass.asm

player60-stereo:
	mads -d:SID2SAPR $(PLAYER_LOWMEM) -d:HZ60 -o:player.xex ../player/asm/playlzs16-stereo.asm

player60-mono:
	mads -d:SID2SAPR -d:HZ60 $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-mono.asm

player60-softbass:
	mads -d:SID2SAPR -d:HZ60 $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-softbass.asm

player60-softbass-sawtooth:
	mads -d:SAWTOOTH -d:SID2SAPR $(PLAYER_LOWMEM) -d:HZ60 -o:player.xex ../player/asm/playlzs16-softbass.asm

player100-stereo:
	mads -d:SID2SAPR $(PLAYER_LOWMEM) -d:HZ100 -o:player.xex ../player/asm/playlzs16-stereo.asm

player100-mono:
	mads -d:SID2SAPR -d:HZ100 $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-mono.asm

player100-softbass:
	mads -d:SID2SAPR -d:HZ100 $(PLAYER_LOWMEM) -o:player.xex ../player/asm/playlzs16-softbass.asm

player100-softbass-sawtooth:
	mads -d:SAWTOOTH -d:SID2SAPR $(PLAYER_LOWMEM) -d:HZ100 -o:player.xex ../player/asm/playlzs16-softbass.asm

$(LZSS):
	+make -C ../lzss-sap

clean:
	rm -f *.exe *.log *~ */*~ sid2sapr

distclean: clean
	rm -f *.sapr *.lz16 player.xex
