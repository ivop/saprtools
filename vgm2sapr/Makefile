
CC ?= gcc

LZSS = ../lzss-sap/bin/lzss

vgm2sapr: main.c
	$(CC) -O2 -o vgm2sapr main.c -lm -lz

compress: $(LZSS)
	@$(LZSS) -6 left.sapr left.lz16   >>convertall.log  2>&1
	@$(LZSS) -6 right.sapr right.lz16 >>convertall.log  2>&1

player50:
	mads -d:VGM2SAPR -o:player.xex ../player/asm/playlzs16-stereo.asm

player60:
	mads -d:VGM2SAPR -d:HZ60 -o:player.xex ../player/asm/playlzs16-stereo.asm

$(LZSS):
	+make -C ../lzss-sap

clean:
	rm -f vgm2sapr *~ */*~

cleaner: clean
	rm -f *.sapr *.xex *.lz16

