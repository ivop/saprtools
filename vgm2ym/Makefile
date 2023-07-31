
CC ?= gcc

vgm2ym: vgm2ym.c
	$(CC) -O2 -o vgm2ym vgm2ym.c -lm -lz

clean:
	rm -f vgm2ym *~ */*~

cleaner: clean
	rm -f *.ym

