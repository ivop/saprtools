CFLAGS ?= -O2 -Wall
CC ?= gcc

sid2sapr: sid2sapr.c sidengine.c
		$(CC) $(CFLAGS) -o sid2sapr sid2sapr.c sidengine.c

clean:
	rm -f *~ sid2sapr
