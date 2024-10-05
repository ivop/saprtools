CC ?= gcc
CFLAGS ?= -std=c99 -D_XOPEN_SOURCE=500 -O2

all: vgm2ym

convertall: vgm2ym
	./convertall.sh

vgm2ym: main.c
	$(CC) $(STATIC) $(CFLAGS) -o vgm2ym main.c -lm -lz

clean:
	rm -f vgm2ym *~ */*~

distclean: clean
	rm -f *.ym

