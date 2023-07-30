
CC ?= gcc

vgmconv: main.c
	$(CC) -O2 -o vgmconv main.c -lm -lz

clean:
	rm -f vgmconv *~ */*~

cleaner: clean
	rm -f *.ym

