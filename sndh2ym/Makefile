CC ?= gcc
BASE = libpsgplay
SRC = $(BASE)/lib/ice/ice.c $(BASE)/lib/internal/print.c \
	$(BASE)/lib/internal/string.c $(BASE)/system/unix/file.c \
	$(BASE)/system/unix/memory.c $(BASE)/system/unix/print.c \
	$(BASE)/system/unix/sndh.c $(BASE)/system/unix/string.c \
	main.c timedb.c

CFLAGS = -O2 -Wall -I$(BASE)/include -std=gnu99
LIB = $(BASE)/lib/psgplay/libpsgplay.a

all: sndh2ym

convertall: sndh2ym
	./convertall.sh

sndh2ym: $(SRC) $(LIB)
	$(CC) $(STATIC) $(CFLAGS) -o sndh2ym $(SRC) $(LIB) -lm

$(LIB):
	+make -C libpsgplay HOST_CC="$(CC)" CC=cc lib/psgplay/libpsgplay.a

clean:
	rm -f *~ sndh2ym sndh2ym.exe
	make -C libpsgplay clean

distclean: clean
	rm -f *.ym
