ifdef DEBUG_BUILD
CFLAGS = -Wall -Wextra -g
LIBS = -lpigpiod_if2 -lrt -lpthread
else
CFLAGS = -Wall -Wextra -O2
LIBS = -lpigpiod_if2 -lrt -lpthread
endif

all: fan control

fan: fan.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

fan.o: fan.c
	gcc $(CFLAGS) -c -o $@ $<

control: control.o
	gcc $(CFLAGS) -o $@ $^

control.o: control.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o fan control
