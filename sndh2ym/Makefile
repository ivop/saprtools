BASE = libpsgplay
SRC = $(BASE)/lib/ice/ice.c $(BASE)/lib/internal/print.c \
	$(BASE)/lib/internal/string.c $(BASE)/system/unix/file.c \
	$(BASE)/system/unix/memory.c $(BASE)/system/unix/print.c \
	$(BASE)/system/unix/sndh.c $(BASE)/system/unix/string.c \
	main.c timedb.c

CFLAGS = -O2 -I$(BASE)/include
LIB = $(BASE)/lib/psgplay/libpsgplay.a

sndh2ym: $(SRC) $(LIB)
	gcc $(CFLAGS) -o sndh2ym $(SRC) $(LIB) -lm

$(LIB):
	make -C libpsgplay lib/psgplay/libpsgplay.a

clean:
	rm -f *~ sndh2ym

cleaner: clean
	make -C libpsgplay clean
	rm -f *.ym
