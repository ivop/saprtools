CFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
#CFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
CFLAGS += $(shell sdl2-config --cflags)
LIBS=mzpokey.a $(shell sdl2-config --static-libs) -lm
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
