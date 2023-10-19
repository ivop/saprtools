CFLAGS ?= -O2 -Wall
CC ?= gcc

CFLAGS += $$(pkg-config --cflags sdl)
LIBS += $$(pkg-config --libs sdl)

CFLAGS += -Ilibatari800/src/libatari800/
LIBS += libatari800/src/libatari800.a

LIBS += -lm

saprplay: main.c libatari800/src/libatari800.a
	$(CC) -o saprplay main.c $(CFLAGS) $(LIBS)

libatari800/src/libatari800.a:
	libatari800/build-libatari800.sh

clean:
	rm -f *~ saprplay

