CXXFLAGS = -g -O2 -pthread -ffast-math -Wall $(shell pkg-config libindi --cflags)
LIBRARIES = $(shell pkg-config libindi --libs)

all: pimount

pimount: pimount.cpp pimount.h
	g++ $(CXXFLAGS) -o $@ $< $(LIBRARIES)

clean:
	rm -f pimount *~ *.o
