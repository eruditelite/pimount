/*
  a4988.c
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pigpiod_if2.h>

#include "a4988.h"

/*
  ------------------------------------------------------------------------------
  _set_mode
*/

static int
_set_mode(int pigpio, unsigned pin, unsigned mode)
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
  _gpio_write
*/

static int
_gpio_write(int pigpio, unsigned pin, unsigned level)
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
  a4988_initialize
*/

int
a4988_initialize(struct a4988 *driver)
{
	int rc = 0;

	rc |= _set_mode(driver->pigpio, driver->sleep, PI_OUTPUT);
	rc |= _gpio_write(driver->pigpio, driver->sleep, 0);
	rc |= _set_mode(driver->pigpio, driver->direction, PI_OUTPUT);
	rc |= _set_mode(driver->pigpio, driver->step, PI_OUTPUT);
	rc |= _set_mode(driver->pigpio, driver->ms1, PI_OUTPUT);
	rc |= _set_mode(driver->pigpio, driver->ms2, PI_OUTPUT);

	if (0 != rc)
		return -1;

	return 0;
}

/*
  ------------------------------------------------------------------------------
  a4988_finalize
*/

void
a4988_finalize(struct a4988 *driver)
{
	_set_mode(driver->pigpio, driver->sleep, PI_INPUT);
	_set_mode(driver->pigpio, driver->direction, PI_INPUT);
	_set_mode(driver->pigpio, driver->step, PI_INPUT);
	_set_mode(driver->pigpio, driver->ms1, PI_INPUT);
	_set_mode(driver->pigpio, driver->ms2, PI_INPUT);

	return;
}

/*
  ------------------------------------------------------------------------------
  a4988_enable
*/

int
a4988_enable(struct a4988 *driver,
	     enum a4988_res resolution, enum a4988_dir direction)
{
	int rc = 0;
	unsigned ms1;
	unsigned ms2;
	struct timespec delay;

	/* Set the MSn Bits */

	switch (resolution) {
	case A4988_RES_FULL:
		ms1 = 0;
		ms2 = 0;
		break;
	case A4988_RES_HALF:
		ms1 = 1;
		ms2 = 0;
		break;
	case A4988_RES_QUARTER:
		ms1 = 0;
		ms2 = 1;
		break;
	case A4988_RES_EIGHTH:
		ms1 = 1;
		ms2 = 1;
		break;
	default:
		fprintf(stderr, "Bad Resolution: %d\n", resolution);
		return -1;
		break;
	}

	rc |= _gpio_write(driver->pigpio, driver->ms1, ms1);
	rc |= _gpio_write(driver->pigpio, driver->ms2, ms2);

	switch (direction) {
	case A4988_DIR_CW:
		rc |= _gpio_write(driver->pigpio, driver->direction, 0);
		break;
	case A4988_DIR_CCW:
		rc |= _gpio_write(driver->pigpio, driver->direction, 1);
		break;
	default:
		fprintf(stderr, "Bad Direction: %d\n", direction);
		return -1;
		break;
	}

	rc |= _gpio_write(driver->pigpio, driver->sleep, 1);

	if (0 != rc)
		return -1;

	/* Wait 1 ms At Least (DS) */
	delay.tv_sec = 0;
	delay.tv_nsec = 1500000;
	nanosleep(&delay, NULL);

	return 0;
}

/*
  ------------------------------------------------------------------------------
  a4988_disable
*/

int
a4988_disable(struct a4988 *driver)
{
	int rc;

	rc = _gpio_write(driver->pigpio, driver->sleep, 0);

	if (0 != rc)
		return -1;

	return 0;
}

/*
  ------------------------------------------------------------------------------
  a4988_step
*/

int
a4988_step(struct a4988 *driver)
{
	int rc = 0;
	struct timespec delay;

	/*
	  Set the Step Pulse Width

	  Without any "special" actions, the minimum GPIO pulse seems
	  to be about 100 us (if you try to delay less than that,
	  you'll still get that, and not very consistently).

	  Also, the gpio_write calls take about 80 us...
	*/

	delay.tv_sec = 0;
	delay.tv_nsec = 100000;

	/* Pulse */
	rc |= _gpio_write(driver->pigpio, driver->step, 1);
	nanosleep(&delay, NULL);
	rc |= _gpio_write(driver->pigpio, driver->step, 0);

	/*
	  Delay at least 2 ms

	  Less than this and the stepper motor just "rattles"...
	*/

	delay.tv_sec = 0;
	delay.tv_nsec = 2000000;
	nanosleep(&delay, NULL);

	if (0 != rc)
		return -1;

	return 0;
}
