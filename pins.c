/*
  pins.c
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <pigpio.h>

#include "pins.h"

/*#define TRACE*/

char *cmdErrStr(int);

/*
  ------------------------------------------------------------------------------
  pins_set_mode
*/

int
pins_set_mode(unsigned pin, unsigned mode)
{
	int rc;

#ifdef TRACE
	printf("%s:%s:%d - pin=%u mode=%u\n",
	       __FILE__, __func__, __LINE__, pin, mode);
#endif

	rc = gpioSetMode(pin, mode);

	if (0 != rc)
		fprintf(stderr, "gpioSetMode(%u,%u) failed: %s\n",
			pin, mode, cmdErrStr(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_set_pull_up_down
*/

int
pins_set_pull_up_down(unsigned pin, unsigned pud)
{
	int rc;

#ifdef TRACE
	printf("%s:%s:%d - pin=%u pud=%u\n",
	       __FILE__, __func__, __LINE__, pin, pud);
#endif

	rc = gpioSetPullUpDown(pin, pud);

	if (0 != rc)
		fprintf(stderr, "gpioSetPullUpDown(%u,%u) failed: %s\n",
			pin, pud, cmdErrStr(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_gpio_read
*/

int
pins_gpio_read(unsigned pin)
{
	int rc;

#ifdef TRACE
	printf("%s:%s:%d - pin=%u\n",
	       __FILE__, __func__, __LINE__, pin, level);
#endif

	rc = gpioRead(pin);

	if (PI_BAD_GPIO == rc) {
		fprintf(stderr, "gpioRead(%u) failed: %s\n",
			pin, cmdErrStr(rc));

		return -1;
	}

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_gpio_write
*/

int
pins_gpio_write(unsigned pin, unsigned level)
{
	int rc;

#ifdef TRACE
	printf("%s:%s:%d - pin=%u level=%u\n",
	       __FILE__, __func__, __LINE__, pin, level);
#endif

	rc = gpioWrite(pin, level);

	if (0 != rc)
		fprintf(stderr, "gpioWrite(%u,%u) failed: %s\n",
			pin, level, cmdErrStr(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_isr
*/

int
pins_isr(unsigned pin, unsigned edge, int timeout, gpioISRFunc_t func)
{
	int rc;

	rc = gpioSetISRFunc(pin, edge, timeout, func);

	if (0 > rc)
		fprintf(stderr, "gpioSetISRFunc(%u,%u,%d, %p) failed: %s\n",
			pin, edge, timeout, func, cmdErrStr(rc));

	return rc;
}
