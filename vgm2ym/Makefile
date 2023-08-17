
CC ?= gcc

vgm2ym: main.c
	$(CC) -O2 -o vgm2ym main.c -lm -lz

clean:
	rm -f vgm2ym *~ */*~

cleaner: clean
	rm -f *.ym

