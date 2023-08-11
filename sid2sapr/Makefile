CFLAGS ?= -O2 -Wall
CC ?= gcc

sid2sapr: main.c sidengine.c
		$(CC) $(CFLAGS) -o sid2sapr main.c sidengine.c

clean:
	rm -f *~ sid2sapr
