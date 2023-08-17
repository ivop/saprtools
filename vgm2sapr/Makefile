
CC ?= gcc

vgm2sapr: main.c
	$(CC) -O2 -o vgm2sapr main.c -lm -lz

clean:
	rm -f vgm2sapr *~ */*~

cleaner: clean
	rm -f *.sapr *.xex

