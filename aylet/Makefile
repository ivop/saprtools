# makefile for aylet

# You need an ANSI C compiler. gcc is probably best.
#
CC ?= gcc

# Compiler options (and sound driver) to use. This one is for Linux,
# and other OSS-using systems.
#
CFLAGS=-O -Wall -DDRIVER_OSS -Wno-pointer-sign -Wno-unused-result

# Set how to link the curses lib - this is for Linux.
# (Most non-Linux systems will probably want `-lcurses' instead.)
#
CURSES_LIB=-lncurses

# dest for make install
#
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
XBINDIR=$(BINDIR)
MANDIR=$(PREFIX)/man/man1

# you shouldn't need to edit the rest
#-----------------------------------------------------------------

# this looks wrong, but *ops.c are actually #include'd by z80.c
OBJS=main.o sound.o ui.o z80.o drv-oss.o

all: aylet

aylet: $(OBJS)
	$(CC) -o aylet $(OBJS) $(CURSES_LIB)

clean:
	$(RM) *.o *~ aylet

# dependencies
cbops.o: cbops.c
drv-oss.o: drv-oss.c
edops.o: edops.c
main.o: main.c main.h sound.h ui.h z80.h
sound.o: sound.c main.h z80.h sound.h driver.h
ui.o: ui.c main.h ui.h
z80.o: z80.c main.h z80.h z80ops.c cbops.c edops.c
z80ops.o: z80ops.c cbops.c edops.c
