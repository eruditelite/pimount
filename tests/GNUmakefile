# Build the Debug Version of tests programs!
DEBUG_BUILD = true

# Common flags.
include ../flags.mk

all: fan input output threads rate client status

status: status.o oled.o stats.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

status.o: status.c
	gcc $(CFLAGS) -c -o $@ $<

fan: fan.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

fan.o: fan.c
	gcc $(CFLAGS) -c -o $@ $<

input: input.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

input.o: input.c
	gcc $(CFLAGS) -c -o $@ $<

output: output.o a4988.o pins.o timespec.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

output.o: output.c
	gcc $(CFLAGS) -c -o $@ $<

threads: threads.o stepper.o a4988.o pins.o timespec.o pimount.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

threads.o: threads.c
	gcc $(CFLAGS) -c -o $@ $<

rate: rate.o a4988.o pins.o timespec.o stepper.o pimount.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

rate.o: rate.c
	gcc $(CFLAGS) -c -o $@ $<

client: client.o
	gcc $(CFLAGS) -o $@ $^

client.o: client.c
	gcc $(CFLAGS) -c -o $@ $<

pimount.o: ../pimount.c ../pimount.h
	gcc $(CFLAGS) -c -o $@ $<

stats.o: ../stats.c ../stats.h
	gcc $(CFLAGS) -c -o $@ $<

oled.o: ../oled.c ../oled.h
	gcc $(CFLAGS) -c -o $@ $<

stepper.o: ../stepper.c ../stepper.h
	gcc $(CFLAGS) -c -o $@ $<

a4988.o: ../a4988.c ../a4988.h
	gcc $(CFLAGS) -c -o $@ $<

pins.o: ../pins.c ../pins.h
	gcc $(CFLAGS) -c -o $@ $<

timespec.o: ../timespec.c ../timespec.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o fan input output threads rate client *.log
