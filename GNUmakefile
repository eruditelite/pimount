# Common flags.
include flags.mk

# Common patterns.
include patterns.mk

SRC = a4988.c fan.c main.c oled.c pimount.c pins.c server.c stats.c \
	stepper.c timespec.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

.PHONY: all cscope clean install

.DEFAULT: all

all: pimount tests indi
	make -C tests all
	make -C indi all

install: all
	sudo cp pimount /usr/local/bin
	sudo cp pimount.service /lib/systemd/system
	sudo cp indi/pimount-indi /usr/local/bin
	sudo cp indi/pimount-indi.service /lib/systemd/system

cscope:
	ls *.c *.h >cscope.files
	cscope -b

pimount: main.o a4988.o pins.o fan.o server.o timespec.o stepper.o \
	oled.o stats.o pimount.o
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	make -C tests clean
	make -C indi clean
	rm -f *~ *.o cscope* pimount cscope.* *.d

-include $(DEP)
