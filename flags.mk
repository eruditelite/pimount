CFLAGS = -Wall -Wextra -pedantic \
	-Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition

ifdef DEBUG_BUILD
CFLAGS += -Og -ggdb3
else
CFLAGS += -O3
endif
