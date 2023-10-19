CFLAGS ?= -O2 -Wall
CC ?= gcc
LIBS = -lm

CFLAGS += $$(pkg-config --cflags sdl)
LIBS += $$(pkg-config --libs sdl)

CFLAGS += -Ilibatari800/src/libatari800/
LIBS += -Llibatari800/src/ -latari800

saprplay: main.c
	$(CC) -o saprplay main.c $(CFLAGS) $(LIBS)

clean:
	rm -f *~ saprplay

