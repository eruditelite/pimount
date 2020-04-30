CFLAGS = -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes \
	-Wold-style-definition -D_GNU_SOURCE=1

ifdef RELEASE_BUILD
CFLAGS += -O3
else
CFLAGS += -Og -ggdb3
endif

LIBS = -lpigpio -lrt -lpthread
