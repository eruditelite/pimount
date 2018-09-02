/*
  pins.c
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pigpiod_if2.h>

#include "pins.h"

/*
  ------------------------------------------------------------------------------
  pins_set_mode
*/

int
pins_set_mode(int pigpio, unsigned pin, unsigned mode)
{
	int rc;

	rc = set_mode(pigpio, pin, mode);

	if (0 != rc)
		fprintf(stderr, "set_mode(%d,%u,%u) failed: %s\n",
			pigpio, pin, mode, pigpio_error(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_set_pull_up_down
*/

int
pins_set_pull_up_down(int pigpio, unsigned pin, unsigned pud)
{
	int rc;

	rc = set_pull_up_down(pigpio, pin, pud);

	if (0 != rc)
		fprintf(stderr, "set_pull_up_down(%d,%u,%u) failed: %s\n",
			pigpio, pin, pud, pigpio_error(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_set_glitch_filter
*/

int
pins_set_glitch_filter(int pigpio, unsigned pin, unsigned steady)
{
	int rc;

	rc = set_glitch_filter(pigpio, pin, steady);

	if (0 != rc)
		fprintf(stderr, "set_glitch_filter(%d,%u,%u) failed: %s\n",
			pigpio, pin, steady, pigpio_error(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_gpio_write
*/

int
pins_gpio_write(int pigpio, unsigned pin, unsigned level)
{
	int rc;

	rc = gpio_write(pigpio, pin, level);

	if (0 != rc)
		fprintf(stderr, "gpio_write(%d,%u,%u) failed: %s\n",
			pigpio, pin, level, pigpio_error(rc));

	return rc;
}

/*
  ------------------------------------------------------------------------------
  pins_callback
*/

int
pins_callback(int pigpio, unsigned pin, unsigned edge,
	      void (*cb)(int, unsigned, unsigned, unsigned))
{
	int rc;

	rc = callback(pigpio, pin, edge, cb);

	if (0 > rc)
		fprintf(stderr, "callback(%d,%u,%u,%p) failed: %d(%s)\n",
			pigpio, pin, edge, cb, rc, pigpio_error(rc));

	return rc;
}
