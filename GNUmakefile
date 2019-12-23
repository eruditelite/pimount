ifdef DEBUG_BUILD
CFLAGS = -Wall -Wextra -g
else
CFLAGS = -Wall -Wextra -O2
endif

LIBS = -lpigpio -lrt -lpthread

all: pimount
	make -C tests all

cscope:
	ls *.c *.h >cscope.files
	cscope -b

tests:
	make -C tests

pimount: main.c a4988.o pins.o fan.o control.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

a4988.o: a4988.c a4988.h
	gcc $(CFLAGS) -c -o $@ $<

pins.o: pins.c pins.h
	gcc $(CFLAGS) -c -o $@ $<

fan.o: fan.c fan.h
	gcc $(CFLAGS) -c -o $@ $<

control.o: control.c control.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	make -C tests clean
	rm -f *~ *.o cscope* pimount
