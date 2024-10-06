CC ?= gcc
CFLAGS ?= -std=c99 -D_XOPEN_SOURCE=500 -O2 -I../zlib

all: psg2ym

convertall: psg2ym
	./convertall.sh

psg2ym: main.c
	$(CC) $(STATIC) $(CFLAGS) -o $@ $< -lm

clean:
	rm -f psg2ym *~ */*~

distclean: clean
	rm -f *.ym
