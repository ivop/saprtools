CXXFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter -flto
#CXXFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter -flto
LIBS=-lm
SRC=main.cpp \
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
	$(CXX) -flto -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

clean:
	rm -f *.o *.a *~ */*~ $(SAPREDIT) $(OBJ)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC) > .depend

include .depend
