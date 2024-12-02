CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
LIBS = -lm

all: sapr2sapr

sapr2sapr: main.c
		$(CC) $(STATIC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *~ sapr2sapr

distclean: clean
	rm -f *.sapr
