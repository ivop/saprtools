CFLAGS ?= -O2 -Wall
CC ?= gcc

tinysid: tinysid.c sidengine.c
		$(CC) $(CFLAGS) -o tinysid tinysid.c sidengine.c

clean:
	rm -f *~ tinysid
