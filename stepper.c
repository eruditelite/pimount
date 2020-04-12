/*
  stepper.c
*/

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <math.h>

#include <pigpio.h>

#include "control.h"
#include "fan.h"
#include "pins.h"
#include "a4988.h"
#include "timespec.h"
#include "stepper.h"

/*
  ==============================================================================
  ==============================================================================
  Private Stuff
  ==============================================================================
  ==============================================================================
*/

/* How close is the same? */
#define SAME_DOUBLE 0.1

char *cmdErrStr(int);

/*
  ------------------------------------------------------------------------------
  stepper_cleanup
*/

static void
stepper_cleanup(void *input)
{
	struct motion *motion;

	motion = (struct motion *)input;
	a4988_finalize(&(motion->driver));
}

static int
stepper_set_ra_rate(struct speed *speed,
		    enum motion_direction direction,
		    double rate)
{
	/*
	  If the speed is 15 arcseconds per second, and the direction
	  is West, track the Earth's speed.  This needs to be very
	  precise!
	*/

	if ((fabs(rate - 15.0) < SAME_DOUBLE) &&
	    (MOTION_DIR_NW == direction)) {
		speed->resolution = A4988_RES_EIGHTH;
		speed->direction = direction;
		speed->width = 500;
		speed->delay = 12000;
		speed->state = MOTION_STATE_ON;
		return EXIT_SUCCESS;
	}

	speed->state = MOTION_STATE_OFF;

	return EXIT_FAILURE;
}

static int
stepper_set_dec_rate(struct speed *speed, double rate)
{
	if (rate <= 30.0) {
		speed->resolution = A4988_RES_HALF;
		speed->width = 500;
		speed->delay = 6000;
	} else {
		speed->resolution = A4988_RES_FULL;
		speed->width = 500;
		speed->delay = 6000;
	}

	speed->state = MOTION_STATE_ON;

	return EXIT_SUCCESS;
}

static int
stepper_get_ra_rate(struct speed *speed, double *rate)
{
	switch (speed->resolution) {
		case A4988_RES_FULL:
			*rate = 0.0;
			break;
		case A4988_RES_HALF:
			*rate = 0.0;
			break;
		case A4988_RES_QUARTER:
			*rate = 0.0;
			break;
		case A4988_RES_EIGHTH:
			*rate = 0.0;
			break;
		default:
			fprintf(stderr, "Invalid Resolution!");
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int
stepper_get_dec_rate(struct speed *speed, double *rate)
{
	switch (speed->resolution) {
		case A4988_RES_FULL:
			*rate = 0.0;
			break;
		case A4988_RES_HALF:
			*rate = 0.0;
			break;
		case A4988_RES_QUARTER:
			*rate = 0.0;
			break;
		case A4988_RES_EIGHTH:
			*rate = 0.0;
			break;
		default:
			fprintf(stderr, "Invalid Resolution!");
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
  */

static int
compare_speeds(struct speed *a, struct speed *b)
{
	if (MOTION_STATE_OFF == a->state &&
	    MOTION_STATE_OFF == b->state)
		return 0;	/* Both off/stopped => the Same */

	if (MOTION_STATE_ON == a->state &&
	    MOTION_STATE_OFF == b->state)
		return 1;	/* a > b (a is on, b is off) */

	if (MOTION_STATE_OFF == a->state &&
	    MOTION_STATE_ON == b->state)
		return -1;	/* b > a (a is off, b is on) */

	/* TODO -- Implement a "get speed" function and use that! */

	if (a->direction == b->direction &&
	    a->resolution == b->resolution &&
	    a->width == b->width &&
	    a->delay == b->delay)
		return 0;

	return 1;
}

/*
  ==============================================================================
  ==============================================================================
  Public Stuff
  ==============================================================================
  ==============================================================================
*/

int
stepper_get_rate(struct speed *speed, enum motion_axis axis, double *rate)
{
	if (MOTION_STATE_OFF == speed->state) {
		/* Motors that are off, shouldn't be moving... */
		*rate = 0.0;
		return EXIT_SUCCESS;
	}

	if (MOTION_AXIS_RA == axis)
		return stepper_get_ra_rate(speed, rate);
	else if (MOTION_AXIS_DEC == axis)
		return stepper_get_dec_rate(speed, rate);

	return EXIT_FAILURE;
}

#if 0
static struct speed ra_speeds[] = {
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 4000},
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 6000},
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 8000},
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 10000},
	{1, A4988_RES_EIGHTH, A4988_DIR_CW, 500, 12000}, /* Earth's Rotation */
	{0, A4988_RES_HALF, A4988_DIR_CCW, 500, 10000},  /* Just stop... */
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 8000},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 6000},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 4000},
};

#define RA_SPEED_DEFAULT 4
#define RA_SPEED_MAX 8

static struct speed dec_speeds[] = {
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 4000},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 6000},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 8000},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 10000},
	{0, A4988_RES_HALF, A4988_DIR_CCW, 500, 0}, /* OFF */
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 100000},
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 8000},
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 6000},
	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 4000},
};

#define DEC_SPEED_DEFAULT 4
#define DEC_SPEED_MAX 8
#endif

int
stepper_set_rate(struct speed *speed,
		 enum motion_state state,
		 enum motion_axis axis,
		 enum motion_direction direction,
		 double rate)
{
	if (MOTION_STATE_OFF == state) {
		speed->state = MOTION_STATE_OFF;
		return EXIT_SUCCESS;
	}

	/*
	  Maximum rate is 8x 15 arcsec/s or 120 arcsec/s.
	*/

	if (rate < 0.0 || rate > 120.0) {
		fprintf(stderr, "Rate Out of Range (0...120)\n");

		return EXIT_FAILURE;
	}

	speed->state = MOTION_STATE_OFF;
	speed->direction = direction;

	if (MOTION_AXIS_RA == axis) {
		int rc;

		rc = stepper_set_ra_rate(speed, direction, rate);

		return rc;
	} else if (MOTION_AXIS_DEC == axis) {
		return stepper_set_dec_rate(speed, rate);
	}

	return EXIT_FAILURE;
}

/*
  ------------------------------------------------------------------------------
  stepper
*/

void *
stepper(void *input)
{
	struct motion *motion;
	struct a4988 *driver;
	struct speed *speed;
	struct timespec period;
	struct timespec sleep;
	struct timespec m[2];
	int mi;
	pthread_t this;
	struct sched_param params;

	motion = (struct motion *)input;
	pthread_mutex_lock(&motion->mutex);
	driver = &motion->driver;
	speed = motion->speed;
	pthread_cleanup_push(stepper_cleanup, input);

 	period.tv_nsec = (speed->width * 1000) + (speed->delay * 1000);
	period = timespec_normalise(period);
	sleep.tv_nsec = (speed->delay * 1000);
	sleep = timespec_normalise(sleep);

	/* Run at a High Priority -- Higher than the Control Thread */
	this = pthread_self();
	params.sched_priority = 75;

	if (0 != pthread_setschedparam(this, SCHED_RR, &params))
		fprintf(stderr, "pthread_setschedparam() failed!\n");

	/* First run... */
	if (1 == speed->state)
		a4988_enable(driver, speed->resolution, speed->direction);

	mi = 0;

	for (;;) {
		struct timespec now;
		struct timespec alarm;
		struct timespec offset;

		/*
		  See if Anything Changed, Update if Needed
		*/
		  
		if (0 != compare_speeds(speed, motion->speed)) {
			a4988_disable(driver);
			speed = motion->speed;

			period.tv_nsec = (speed->width * 1000) +
				(speed->delay * 1000);
			period = timespec_normalise(period);
			sleep.tv_nsec = (speed->delay * 1000);
			sleep = timespec_normalise(sleep);

			if (1 == speed->state)
				a4988_enable(driver, speed->resolution,
					     speed->direction);
		}

		pthread_testcancel();

		/* POSIX will only work with CLOCK_REALTIME, */
		clock_gettime(CLOCK_REALTIME, &now);

		if (1 == speed->state) {
			a4988_step(driver, speed->width);

			/* but offset and sleep are just relative. */
			clock_gettime(CLOCK_MONOTONIC, &m[mi++]);

			if (2 == mi) {
				offset = timespec_sub(m[1], m[0]);
				offset = timespec_normalise(offset);

				if (timespec_gt(period, offset)) {
					offset = timespec_sub(period, offset);
					sleep = timespec_add(sleep, offset);
				} else {
					offset = timespec_sub(offset, period);
					sleep = timespec_sub(sleep, offset);
				}

				mi = 0;
			}

			/* calculate the time to wake up */
			alarm = now;
			alarm = timespec_add(alarm, sleep);
			alarm = timespec_normalise(alarm);
		} else {
			alarm.tv_sec = now.tv_sec + 1;
			alarm.tv_nsec = now.tv_nsec;
		}

		pthread_cond_timedwait(&motion->cond, &motion->mutex, &alarm);
	}

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}
