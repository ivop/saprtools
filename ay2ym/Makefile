CC ?= gcc
CFLAGS ?= -O2 -Wall

all: ay2ym

ay2ym: main.c z80.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f ay2ym *.o *~
