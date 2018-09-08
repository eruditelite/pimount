ifdef DEBUG_BUILD
CFLAGS = -Wall -Wextra -g
LIBS = -lpigpiod_if2 -lrt -lpthread
else
CFLAGS = -Wall -Wextra -O2
LIBS = -lpigpiod_if2 -lrt -lpthread
endif

all: pimount

pimount: main.c a4988.o pins.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

a4988.o: a4988.c a4988.h
	gcc $(CFLAGS) -c -o $@ $<

pins.o: pins.c pins.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ pins.o a4988.o pimount
