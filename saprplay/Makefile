CFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
#CFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
CFLAGS += $$(pkg-config --cflags sdl)
LIBS=mzpokeysnd.a $$(pkg-config --libs sdl) -lm

all: saprplay mzpokeysnd.a

mzpokeysnd.a: mzpokeysnd.o remez.o
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

saprplay: mzpokeysnd.a main.o
	$(CC) -o $@ $^ $(LIBS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f saprplay *.o *.a *~
