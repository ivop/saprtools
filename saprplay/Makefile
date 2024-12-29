CFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
#CFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
CFLAGS += $$(pkg-config --cflags sdl2)
LIBS=mzpokey.a $$(pkg-config --libs sdl2) -lm
SRC_FILES=mzpokey.c remez.c main.c

all: saprplay mzpokey.a

mzpokey.a: mzpokey.o remez.o
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

saprplay: mzpokey.a main.o
	$(CC) -s -o $@ $^ $(LIBS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f saprplay *.o *.a *~

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC_FILES) > .depend

include .depend
