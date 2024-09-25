CC ?= gcc
CFLAGS ?= -O2 -Wall

all: ay2ym

convertall: ay2ym
	./convertall.sh

ay2ym: main.c z80.c
	$(CC) $(CFLAGS) $(STATIC) -o $@ $^

clean:
	rm -f ay2ym *.o *~
