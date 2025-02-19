CXXFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter -flto
#CXXFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter -flto
LIBS=-lm
SRC=sapredit.cpp \
	mzpokey.cpp \
	remez.cpp
OBJ=$(SRC:.cpp=.o)
LIBS=
SAPREDIT=sapredit

#include warnings.mk

CXXFLAGS += $(shell fltk-config --use-images --cflags)
CXXFLAGS += $(shell sdl2-config --cflags)
LIBS += $(shell fltk-config --use-images --ldstaticflags)
LIBS += $(shell sdl2-config --static-libs)

all: $(SAPREDIT)

$(SAPREDIT): $(OBJ)
	$(CXX) $(STATIC) -flto -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

icons.inc:
	xxd -i png/fwd.png > $@
	xxd -i png/pause.png >> $@
	xxd -i png/play.png >> $@
	xxd -i png/rew.png >> $@
	xxd -i png/stop.png >> $@

clean:
	rm -f *.o *.a *~ */*~ $(SAPREDIT) $(OBJ)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC) > .depend

include .depend
