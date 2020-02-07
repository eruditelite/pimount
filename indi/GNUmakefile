CXXFLAGS = -g -O2 -pthread -ffast-math -Wall -I /usr/include/libindi
LIBRARIES = -lindi -lindidriver

all: pimount

pimount: pimount.cpp pimount.h
	g++ $(CXXFLAGS) -o $@ $< $(LIBRARIES)

clean:
	rm -f pimount *~ *.o
