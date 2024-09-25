CC ?= gcc

all: vgm2ym

convertall: vgm2ym
	./convertall.sh

vgm2ym: main.c
	$(CC) $(STATIC) -O2 -o vgm2ym main.c -lm -lz

clean:
	rm -f vgm2ym *~ */*~

distclean: clean
	rm -f *.ym

