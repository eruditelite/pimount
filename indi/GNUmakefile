CXXFLAGS = -g -O2 -pthread -ffast-math -Wall $(shell pkg-config libindi --cflags)
LIBRARIES = $(shell pkg-config libindi --libs)

all: pimount-indi

pimount-indi: pimount-indi.cpp pimount-indi.h
	g++ $(CXXFLAGS) -o $@ $< $(LIBRARIES)

clean:
	rm -f pimount-indi *~ *.o
