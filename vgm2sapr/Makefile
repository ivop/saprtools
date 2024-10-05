CC ?= gcc
LZSS ?= ../lzss-sap/bin/lzss
CFLAGS ?= -std=c99 -D_XOPEN_SOURCE=500 -O2 -I../zlib
ZLIB ?= ../zlib/libz.a

all: vgm2sapr

convertall: vgm2sapr
	./convertall.sh

$(ZLIB):
	+cd ../zlib && ./configure --static && make libz.a

vgm2sapr: main.c $(ZLIB)
	$(CC) $(STATIC) $(CFLAGS) -o vgm2sapr main.c -lm $(ZLIB)

compress-stereo: $(LZSS)
	@$(LZSS) -6 left.sapr left.lz16   >>convertall.log  2>&1
	@$(LZSS) -6 right.sapr right.lz16 >>convertall.log  2>&1

player50-stereo:
	mads -d:VGM2SAPR -o:player.xex ../player/asm/playlzs16-stereo.asm

player60-stereo:
	mads -d:VGM2SAPR -d:HZ60 -o:player.xex ../player/asm/playlzs16-stereo.asm

$(LZSS):
	+make -C ../lzss-sap

clean:
	rm -f *.exe *.log *~ */*~ vgm2sapr

distclean: clean
	rm -f *.sapr *.xex *.lz16

