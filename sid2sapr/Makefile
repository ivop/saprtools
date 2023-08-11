CFLAGS ?= -O2 -Wall
CC ?= gcc
LIBS = -lm

sid2sapr: main.c sidengine.c
		$(CC) $(CFLAGS) -o sid2sapr main.c sidengine.c $(LIBS)

clean:
	rm -f *~ sid2sapr
