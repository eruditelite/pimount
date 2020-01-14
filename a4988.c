/*
  a4988.c
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <pigpio.h>

#include "a4988.h"
#include "pins.h"
#include "timespec.h"

/*
  ------------------------------------------------------------------------------
  a4988_initialize
*/

int
a4988_initialize(struct a4988 *driver)
{
	int rc = 0;

	rc |= pins_set_mode(driver->sleep, PI_OUTPUT);
	rc |= pins_gpio_write(driver->sleep, 0);
	rc |= pins_set_mode(driver->direction, PI_OUTPUT);
	rc |= pins_set_mode(driver->step, PI_OUTPUT);
	rc |= pins_set_mode(driver->ms1, PI_OUTPUT);
	rc |= pins_set_mode(driver->ms2, PI_OUTPUT);

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
	pins_set_mode(driver->sleep, PI_INPUT);
	pins_set_mode(driver->direction, PI_INPUT);
	pins_set_mode(driver->step, PI_INPUT);
	pins_set_mode(driver->ms1, PI_INPUT);
	pins_set_mode(driver->ms2, PI_INPUT);

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

	rc |= pins_gpio_write(driver->ms1, ms1);
	rc |= pins_gpio_write(driver->ms2, ms2);

	switch (direction) {
	case A4988_DIR_CW:
		rc |= pins_gpio_write(driver->direction, 0);
		break;
	case A4988_DIR_CCW:
		rc |= pins_gpio_write(driver->direction, 1);
		break;
	default:
		fprintf(stderr, "Bad Direction: %d\n", direction);
		return -1;
		break;
	}

	rc |= pins_gpio_write(driver->sleep, 1);

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

	rc = pins_gpio_write(driver->sleep, 0);

	if (0 != rc)
		return -1;

	return 0;
}

/*
  ------------------------------------------------------------------------------
  a4988_step

  width is in us.
*/

int
a4988_step(struct a4988 *driver, unsigned width)
{
	int rc = 0;
	struct timespec delay;
	static int first_call = 1;
	static struct timespec last;
	struct timespec pre;
	struct timespec post;
	struct timespec diff;
	static unsigned offset = 0;

	/*
	  If called with 2 ms of the last pulse, the motor just
	  "rattles".  So, make sure it's been at least 2 ms since the
	  last pulse.
	*/

	if (!first_call) {
		clock_gettime(CLOCK_MONOTONIC, &diff);
		diff = timespec_sub(diff, last);

		if ((diff.tv_sec == 0) &&
		    (diff.tv_nsec < (2 * 1000 * 1000))) {
			diff.tv_nsec = (2 * 1000 * 1000);
			nanosleep(&diff, NULL);
		}
	}

	/*
	  Set the Step Pulse Width

	  Without any "special" actions, the minimum GPIO pulse seems
	  to be about 100 us (if you try to delay less than that,
	  you'll still get that, and not very consistently).

	  Also, the gpio_write calls take about 80 us...

	  To "correct" for system call times etc., measure the actual
	  time of each pulse and update an "offset" value.
	*/

	/* calculate the delay (using the offset mentioned above) */
	delay.tv_sec = 0;
	delay.tv_nsec = (width * 1000) - offset;

	if (delay.tv_nsec < (100 * 1000)) /* minimum delay */
		delay.tv_nsec = (100 * 1000);

	/* Pulse */
	clock_gettime(CLOCK_MONOTONIC, &pre);
	rc |= pins_gpio_write(driver->step, 1);
	nanosleep(&delay, NULL);
	rc |= pins_gpio_write(driver->step, 0);
	clock_gettime(CLOCK_MONOTONIC, &post);

	/* update offset */
	diff = timespec_sub(post, pre);
	delay.tv_sec = 0;
	delay.tv_nsec = width * 1000;

	if (timespec_gt(diff, delay)) {
		diff = timespec_sub(diff, delay);
		offset += diff.tv_nsec;
	} else {
		diff = timespec_sub(delay, diff);
		offset -= diff.tv_nsec;
	}

	/* if this is the first call, it isn't anymore... */
	if (first_call)
		first_call = 0;

	/*
	  Record the time as the last pulse.
	*/

	clock_gettime(CLOCK_MONOTONIC, &last);

	if (0 != rc)
		return -1;

	return 0;
}
