CC ?= gcc
CFLAGS ?= -std=c99 -D_XOPEN_SOURCE=500 -O2 -I../zlib
ZLIB ?= ../zlib/libz.a

all: vgm2ym

convertall: vgm2ym
	./convertall.sh

$(ZLIB):
	+cd ../zlib && ./configure --static && make libz.a

vgm2ym: main.c $(ZLIB)
	$(CC) $(STATIC) $(CFLAGS) -o vgm2ym main.c -lm $(ZLIB)

clean:
	rm -f vgm2ym *~ */*~

distclean: clean
	rm -f *.ym

