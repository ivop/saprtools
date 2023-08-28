CFLAGS ?= -O2 -Wall
CC ?= gcc
LIBS = -lm
LZSS = ../lzss-sap/bin/lzss

sid2sapr: main.c c64.c m6502.h c64.h md5.c md5.h
		$(CC) $(CFLAGS) -o sid2sapr main.c c64.c md5.c $(LIBS)

compress-mono: $(LZSS)
	$(LZSS) -6 output.sapr output.lz16

compress-softbass: compress-mono

player-mono:
	mads -d:SID2SAPR -o:player.xex asm/playlzs16-mono.asm

player60-mono:
	mads -d:SID2SAPR -d:HZ60 -o:player.xex asm/playlzs16-mono.asm

player-softbass:
	mads -d:SID2SAPR -o:player.xex asm/playlzs16-softbass.asm

player60-softbass:
	mads -d:SID2SAPR -d:HZ60 -o:player.xex asm/playlzs16-softbass.asm

$(LZSS):
	+make -C ../lzss-sap

clean:
	rm -f *~ sid2sapr */*~

cleaner: clean
	rm -f *.sapr *.lz16 player.xex
