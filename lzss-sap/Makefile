CC?=gcc
CFLAGS?=-O2 -Wall

PROGS= lz4s lzss split

all: $(PROGS:%=bin/%)

bin/%: src/%.c | bin
	$(CC) $(STATIC) -o $@ $(CFLAGS) $<

bin:
	mkdir -p bin

clean:
	rm -f *~ */*~
	rm -rf bin

