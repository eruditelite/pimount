ifdef DEBUG_BUILD
CFLAGS = -Wall -Wextra -g
else
CFLAGS = -Wall -Wextra -O2
endif

LIBS = -lpigpio -lrt

all: control input fan

control: control.o
	gcc $(CFLAGS) -o $@ $^

control.o: control.c
	gcc $(CFLAGS) -c -o $@ $<

fan: fan.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

fan.o: fan.c
	gcc $(CFLAGS) -c -o $@ $<

input: input.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

input.o: input.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o fan control input
