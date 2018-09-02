CFLAGS = -Wall -Wextra -O2

all: pimount

pimount: main.c a4988.o pins.o
	gcc $(CFLAGS) -o $@ $^ -lpigpiod_if2 -lrt

a4988.o: a4988.c a4988.h
	gcc $(CFLAGS) -c -o $@ $<

pins.o: pins.c pins.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ pins.o a4988.o pimount
