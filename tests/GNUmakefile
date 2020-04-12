include ../flags.mk

LIBS = -lpigpio -lrt -lpthread

all: control input rate output fan

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

rate: rate.o a4988.o pins.o timespec.o stepper.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

rate.o: rate.c
	gcc $(CFLAGS) -c -o $@ $<

stepper.o: ../stepper.c ../stepper.h
	gcc $(CFLAGS) -c -o $@ $<

output: output.o a4988.o pins.o timespec.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

output.o: output.c
	gcc $(CFLAGS) -c -o $@ $<

a4988.o: ../a4988.c ../a4988.h
	gcc $(CFLAGS) -c -o $@ $<

pins.o: ../pins.c ../pins.h
	gcc $(CFLAGS) -c -o $@ $<

timespec.o: ../timespec.c ../timespec.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o fan control input output
