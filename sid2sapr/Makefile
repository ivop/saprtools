FLAGS ?= -O2 -Wall
LIBS ?= -lpthread
CC ?= gcc

tinysid: tinysid.c sidengine.c soundcard.c
		$(CC) $(FLAGS) -o tinysid tinysid.c sidengine.c soundcard.c $(LIBS)

clean:
	rm -f *~ tinysid
