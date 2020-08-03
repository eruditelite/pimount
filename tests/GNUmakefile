# Build the Debug Version of tests programs!
DEBUG_BUILD = true

# Common flags.
include ../flags.mk

# Common patterns.
include ../patterns.mk

SRC = client.c fan.c input.c output.c rate.c status.c threads.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

.PHONY: all cscope clean

.DEFAULT: all

all: fan input output threads rate client status

status: status.o ../oled.o ../stats.o ../pimount.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

fan: fan.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

input: input.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

output: output.o ../a4988.o ../pins.o ../timespec.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

threads: threads.o ../stepper.o ../a4988.o ../pins.o ../timespec.o ../pimount.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

rate: rate.o ../a4988.o ../pins.o ../timespec.o ../stepper.o ../pimount.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

client: client.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f *~ *.o fan input output threads rate client *.log *.d

-include $(DEP)
