/*
  ==============================================================================
  stepper.c

  API for RA and DEC steppers.
  ==============================================================================
*/

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
#include <errno.h>

#include <pigpio.h>

#include "pimount.h"
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

struct stepper_parameters {
	pthread_mutex_t mutex;

	enum stepper_state state;
	enum stepper_direction direction;
	double rate;
	long int duration;	/* in milli seconds */
	long int remaining;	/* in milli seconds */

	/* A4988 Stuff */
	struct {
		struct a4988 driver;
		enum a4988_res resolution;
		enum a4988_dir direction;
	} a4988;

	/* Timing */
	long width;		/* in micro seconds */
	long delay;		/* in micro seconds */
};

struct stepper {
	pthread_mutex_t mutex;
	bool initialized;

	pthread_t ra_thread;
	struct stepper_parameters ra_parameters;

	pthread_t dec_thread;
	struct stepper_parameters dec_parameters;
};

static struct stepper global = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.initialized = false
};

/* How close is the same? */
#define SAME_DOUBLE 0.1

char *cmdErrStr(int);

#undef STEPPER_TRACE
/*#define STEPPER_TRACE*/

#ifdef STEPPER_TRACE
#define TRACES 200

static struct timespec trace_period;

struct trace {
	struct timespec now;
	int mib;
	int mia;
	struct timespec mb[2];
	struct timespec ma[2];
	struct timespec offsetb;
	struct timespec offseta;
	struct timespec sleepb;
	struct timespec sleepa;
};

static struct trace traces[TRACES];
static int traces_i;

static inline void
copy_timespec(struct timespec *to, struct timespec *from)
{
	to->tv_sec = from->tv_sec;
 	to->tv_nsec = from->tv_nsec;
}

static void
display_trace(void)
{
	int i;
	struct trace *t;

	t = &traces[0];

	printf("-- Requested Period is {%ld %ld} -- \n",
	       trace_period.tv_sec, trace_period.tv_nsec);

	for (i = 0; i < traces_i; ++i) {
		printf("-- Iteration %d --\n"
		       "\tnow={%ld %ld}\n"
		       "\tmib=%d mia=%d\n"
		       "\tmb={{%ld %ld}, {%ld %ld}} ma={{%ld %ld}, {%ld %ld}}\n"
		       "\toffsetb={%ld %ld} offseta={%ld %ld}\n"
		       "\tsleepb={%ld %ld} sleepa={%ld %ld}\n",
		       (i + 1), t->now.tv_sec, t->now.tv_nsec, t->mib, t->mia,
		       t->mb[0].tv_sec, t->mb[0].tv_nsec,
		       t->mb[1].tv_sec, t->mb[1].tv_nsec,
		       t->ma[0].tv_sec, t->ma[0].tv_nsec,
		       t->ma[1].tv_sec, t->ma[1].tv_nsec,
		       t->offsetb.tv_sec, t->offsetb.tv_nsec,
		       t->offseta.tv_sec, t->offseta.tv_nsec,
		       t->sleepb.tv_sec, t->sleepb.tv_nsec,
		       t->sleepa.tv_sec, t->sleepa.tv_nsec);

		++t;
	}
}

#endif	/* STEPPER_TRACE */

/*
  ------------------------------------------------------------------------------
  ra_update_from_rate

  Set the A4988 values in the struction based on rate and direction.

  A pulse width of 500 us is used in all cases, with the resolution
  and delay between pulses varying to get to the requested rate (in
  arc-seconds per second).  The minimum delay is 2000 us (see the
  a4988 driver).

  For the RA axis, maximum rate the original controller allows is 8x
  the tracking rate, or (15 * 8) arc-seconds per second -- 120
  arc-seconds per second.

  Based on the measurements below, the formula for rate and delay is
  as follows.

       rate = ([resolution] * 1,440,000) / delay

       Where resolution is 1/8, 1/4, 1/2, or 1 and delay is in us.
       The rate is in arc-seconds per second.

  Measurements were made using the setting rings.

  Resolution would be 1/8 up to 60 as/s (arc seconds per second) with delays of

       - 96000 us for 1.875 as/s
       - 48000 us for 3.75 as/s
       - 24000 us for 7.5 as/s
       - 12000 us for 15 as/s
       -  6000 us for 30 as/s
       -  3000 us for 60 as/s

  At 1/4,

       - 12000 us for 30 as/s

  At 1/2,

       - 24000 us for 30 as/s

  At full,

       - 48000 us for 30 as/s

  UPDATE

  The originall 1440000 is a bit fast based on tracking during
  observation.  To make adjustments easier, define THE_RA_NUMBER and
  adjust from there...
*/

#define THE_RA_NUMBER 1460000

static int
ra_update_from_rate(struct stepper_parameters *sp)
{
	/*
	  Use 500 us for width, and set the direction.
	*/

	sp->width = 500;

	if (0.0 < sp->rate)
		sp->a4988.direction = A4988_DIR_CW;
	else
		sp->a4988.direction = A4988_DIR_CCW;

	/*
	  Based on the above, use a resolution of 1/8 for rates from
	  -30.0 to 30.0 as/s.
	*/

	if (30.0 >= fabs(sp->rate)) {
		sp->a4988.resolution = A4988_RES_EIGHTH;
		sp->delay = (THE_RA_NUMBER / 8) / sp->rate;

		return EXIT_SUCCESS;
	}

	/*
	  Between 30.0 and 60.0 (or -60.0 and -30.0), use 1/4.
	*/

	if (60.0 >= fabs(sp->rate)) {
		sp->a4988.resolution = A4988_RES_QUARTER;
		sp->delay = (THE_RA_NUMBER / 4) / sp->rate;

		return EXIT_SUCCESS;
	}

	/*
	  Between 60.0 and 120.0 (or -120.0 and -60.0), use 1.
	*/

	if (120.0 >= fabs(sp->rate)) {
		sp->a4988.resolution = A4988_RES_FULL;
		sp->delay = THE_RA_NUMBER / sp->rate;

		return EXIT_SUCCESS;
	}

	fprintf(stderr,	"%s:%d - Requested rate is out of range: %.2f\n",
		__FILE__, __LINE__, sp->rate);

	return EXIT_FAILURE;
}

/*
  ------------------------------------------------------------------------------
  dec_update_from_rate

  Set the A4988 values in the struction based on rate and direction.

  As above, use 500 us as the pulse width and adjust resolution and
  delay to control the rate.

  Note that 15 asec/sec is .25 degrees per minute

  Using the setting circle does not allow for much precision, but it
  seems like the end result is the same as in the RA case -- even
  though the gearing looks quite different (there is a gear box on the
  RA motor, but not on the DEC motor).

  UPDATE

  Not even close to RA... Use THE_DEC_NUMBER below to set the value.
  Need to use observational measurements to find the number.
*/

#define THE_DEC_NUMBER 2840000

static int
dec_update_from_rate(struct stepper_parameters *sp)
{
	/*
	  Use 500 us for width, and set the direction.
	*/

	sp->width = 500;

	if (0.0 < sp->rate)
		sp->a4988.direction = A4988_DIR_CW;
	else
		sp->a4988.direction = A4988_DIR_CCW;

	/*
	  Based on the above, use a resolution of 1/8 for rates from
	  -30.0 to 30.0 as/s.
	*/

	if (30.0 >= fabs(sp->rate)) {
		sp->a4988.resolution = A4988_RES_EIGHTH;
		sp->delay = (THE_DEC_NUMBER / 8) / sp->rate;

		return EXIT_SUCCESS;
	}

	/*
	  Between 30.0 and 60.0 (or -60.0 and -30.0), use 1/4.
	*/

	if (60.0 >= fabs(sp->rate)) {
		sp->a4988.resolution = A4988_RES_QUARTER;
		sp->delay = (THE_DEC_NUMBER / 4) / sp->rate;

		return EXIT_SUCCESS;
	}

	/*
	  Between 60.0 and 120.0 (or -120.0 and -60.0), use 1.
	*/

	if (120.0 >= fabs(sp->rate)) {
		sp->a4988.resolution = A4988_RES_FULL;
		sp->delay = THE_DEC_NUMBER / sp->rate;

		return EXIT_SUCCESS;
	}

	fprintf(stderr,	"%s:%d - Requested rate is out of range: %.2f\n",
		__FILE__, __LINE__, sp->rate);

	return EXIT_FAILURE;
}

/*
  ------------------------------------------------------------------------------
  stepper
*/

static void
stepper_cleanup(void *input)
{
	struct stepper_parameters *sp;

	sp = (struct stepper_parameters *)input;

	a4988_disable(&sp->a4988.driver);
	sp->state = STEPPER_STATE_OFF;
	sp->remaining = 0;
#ifdef STEPPER_TRACE
	display_trace();
#endif	/* STEPPER_TRACE */

	return;
}

static void *
stepper(void *input)
{
	int rc;
	struct stepper_parameters *sp;
	pthread_t this;
	struct sched_param params;
	struct timespec period;
	struct timespec sleep;
	bool run_forever;
	struct timespec stop;
	sp = (struct stepper_parameters *)input;

	rc = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setcancelstate() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		pthread_exit(NULL);
	}

	/* Run at a High Priority -- Higher than the Control Thread */
	this = pthread_self();
	params.sched_priority = 75;

	rc = pthread_setschedparam(this, SCHED_RR, &params);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setschedparam() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));
		pthread_exit(NULL);
	}

	rc = a4988_enable(&sp->a4988.driver,
			  sp->a4988.resolution, sp->a4988.direction);

	if (rc) {
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
		pthread_exit(NULL);
	}

#ifdef STEPPER_TRACE
	traces_i = 0;
	memset(traces, 0, sizeof(traces));
#endif	/* STEPPER_TRACE */

	/*
	  Initialize the Period and Sleep

	  Make sure the conversion from milli seconds to nano seconds
	  does not overflow!  Use the GCC (version >= 5) built-ins to
	  do this.
	*/

	period.tv_sec = 0;

 	if (__builtin_smull_overflow((sp->width + sp->delay), 1000,
				     &period.tv_nsec)) {
		fprintf(stderr, "%s:%d - OVERFLOW calculating period\n",
			__FILE__, __LINE__);
		pthread_exit(NULL);
	}

	period = timespec_normalise(period);

#ifdef STEPPER_TRACE
	copy_timespec(&trace_period, &period);
#endif	/* STEPPER_TRACE */

	sleep.tv_sec = 0;

 	if (__builtin_smull_overflow(sp->delay, 1000, &sleep.tv_nsec)) {
		fprintf(stderr, "%s:%d - OVERFLOW calculating sleep\n",
			__FILE__, __LINE__);
		pthread_exit(NULL);
	}

	sleep = timespec_normalise(sleep);

	/* When will it be time to stop? */
	if (0 == sp->duration)
		run_forever = true;
	else
		run_forever = false;

	if (!run_forever) {
		clock_gettime(CLOCK_REALTIME, &stop);
		stop = timespec_add(stop, timespec_from_ms(sp->duration));
		stop = timespec_normalise(stop);
	} else {
		pthread_mutex_lock(&sp->mutex);
		sp->remaining = 0;
		pthread_mutex_unlock(&sp->mutex);
	}

	pthread_cleanup_push(stepper_cleanup, sp);

	rc = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setcanceltype() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		pthread_exit(NULL);
	}

	rc = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setcancelstate() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		pthread_exit(NULL);
	}

	/*
	  This is the main loop -- now that everything has been set up.
	*/

	for (;;) {
		static int mi = 0;
		static struct timespec m[2];
		struct timespec now;
		struct timespec offset;

		/* Check for Cancellation */
		pthread_testcancel();

		if (!run_forever) {
			clock_gettime(CLOCK_REALTIME, &now);
			if (timespec_gt(now, stop))
				break;

			pthread_mutex_lock(&sp->mutex);
			sp->remaining = timespec_to_ms(timespec_sub(stop, now));
			if (0 == sp->remaining)
				sp->remaining = 1;
			pthread_mutex_unlock(&sp->mutex);
		}

		/* POSIX will only work with CLOCK_REALTIME, */
		clock_gettime(CLOCK_REALTIME, &now);

#ifdef STEPPER_TRACE
		copy_timespec(&traces[traces_i].now, &now);
		traces[traces_i].mib = mi;
		copy_timespec(&traces[traces_i].mb[0], &m[0]);
		copy_timespec(&traces[traces_i].mb[1], &m[1]);
#endif	/* STEPPER_TRACE */

		/* Step the Stepper */
		a4988_step(&sp->a4988.driver, sp->width);

		/* offset and sleep are just relative. */
		clock_gettime(CLOCK_MONOTONIC, &m[mi++]);

#ifdef STEPPER_TRACE
		traces[traces_i].mia = mi;
		copy_timespec(&traces[traces_i].ma[0], &m[0]);
		copy_timespec(&traces[traces_i].ma[1], &m[1]);
#endif	/* STEPPER_TRACE */

		if (2 == mi) {
			offset = timespec_sub(m[1], m[0]);
			offset = timespec_normalise(offset);
#ifdef STEPPER_TRACE
			copy_timespec(&traces[traces_i].offsetb, &offset);
			copy_timespec(&traces[traces_i].sleepb, &sleep);
#endif	/* STEPPER_TRACE */

			offset = timespec_sub(period, offset);
			offset = timespec_from_ms((timespec_to_ms(offset) / 4));

			if (timespec_gt(sleep, offset))
				sleep = timespec_add(sleep, offset);

			if (0 > timespec_to_ms(sleep)) {
				sleep.tv_sec = 0;
				sleep.tv_nsec = sp->delay * 1000;
				sleep = timespec_normalise(sleep);
			}					

#ifdef STEPPER_TRACE
			copy_timespec(&traces[traces_i].offseta, &offset);
			copy_timespec(&traces[traces_i].sleepa, &sleep);
#endif	/* STEPPER_TRACE */

			mi = 0;
		}

		rc = nanosleep(&sleep, NULL);

		if (rc) {
			fprintf(stderr,	"%s:%d - nanosleep(%ld %ld) failed: %s\n",
				__FILE__, __LINE__,
				sleep.tv_sec, sleep.tv_nsec, strerror(errno));
		}

#ifdef STEPPER_TRACE
		++traces_i;
		if (TRACES == traces_i)
			break;
#endif	/* STEPPER_TRACE */
	}

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

/*
  ------------------------------------------------------------------------------
  init_state
*/

static int
init_state(enum stepper_axis axis,
	   struct stepper_parameters *sp, const char *description)
{
	int rc;

	rc = pthread_mutex_init(&sp->mutex, NULL);

	if (0 != rc) {
		fprintf(stderr,	"%s:%d - pthread_mutex_init() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		return -1;
	}

	sp->state = STEPPER_STATE_OFF;

	if (STEPPER_AXIS_RA == axis) {
		sp->a4988.driver.direction = RA_PIN_DIRECTION;
		sp->a4988.driver.step = RA_PIN_STEP;
		sp->a4988.driver.sleep = RA_PIN_SLEEP;
		sp->a4988.driver.ms2 = RA_PIN_MS2;
		sp->a4988.driver.ms1 = RA_PIN_MS1;
	} else if (STEPPER_AXIS_DEC == axis) {
		sp->a4988.driver.direction = DEC_PIN_DIRECTION;
		sp->a4988.driver.step = DEC_PIN_STEP;
		sp->a4988.driver.sleep = DEC_PIN_SLEEP;
		sp->a4988.driver.ms2 = DEC_PIN_MS2;
		sp->a4988.driver.ms1 = DEC_PIN_MS1;
	}

	strncpy(sp->a4988.driver.description,
		description, A4988_DESCRIPTION_SIZE);

	if (a4988_initialize(&(sp->a4988.driver))) {
		fprintf(stderr, "%s:%d - a4988_initialize() failed!\n",
			__FILE__, __LINE__);
		pthread_mutex_destroy(&sp->mutex);

		return -1;
	}

	return 0;
}

/*
  ==============================================================================
  ==============================================================================
  Public Stuff
  ==============================================================================
  ==============================================================================
*/

/*
  ------------------------------------------------------------------------------
  stepper_initialize
*/

int
stepper_initialize(void)
{
	/* Lock */

	lock(&global.mutex);

	if (true == global.initialized) {
		fprintf(stderr,
			"%s:%d - Already Initialized\n", __FILE__, __LINE__);

		return -1;
	}

	/*
	  Initialize the RA and DEC States
	*/

	if (init_state(STEPPER_AXIS_RA, &global.ra_parameters, "pimount.ra")) {
		unlock(&global.mutex);
		fprintf(stderr, "%s:%d - init_state() failed!\n",
			__FILE__, __LINE__);

		return -1;
	}

	if (init_state(STEPPER_AXIS_DEC, &global.dec_parameters, "pimount.dec")) {
		unlock(&global.mutex);
		fprintf(stderr, "%s:%d - init_state() failed!\n",
			__FILE__, __LINE__);

		return -1;
	}

	/* Update Globals and Unlock */

	global.initialized = true;
	unlock(&global.mutex);

	/* Report the Results */

	printf("Initialized Steppers\n");

	return 0;
}

/*
  ------------------------------------------------------------------------------
  stepper_finalize
*/

void
stepper_finalize(void)
{
	/* Lock	*/

	lock(&global.mutex);

	/*
	  The Main Part
	*/

	if (!global.initialized) {
		fprintf(stderr,
			"%s:%d - Already Uninitialized!\n", __FILE__, __LINE__);

		return;
	}

	unlock(&global.mutex);

	stepper_stop(STEPPER_AXIS_RA);
	a4988_finalize(&global.ra_parameters.a4988.driver);

	stepper_stop(STEPPER_AXIS_DEC);
	a4988_finalize(&global.dec_parameters.a4988.driver);

	lock(&global.mutex);

	/* Update Globals */

	global.initialized = false;

	/* Clear Parameters */

	memset(&global.ra_parameters, 0, sizeof(struct stepper_parameters));
	memset(&global.dec_parameters, 0, sizeof(struct stepper_parameters));

	/* Unlock */

	unlock(&global.mutex);

	/* Report the Results */

	printf("Finalized Steppers\n");

	return;
}

/*
  ------------------------------------------------------------------------------
  stepper_start
*/

int
stepper_start(enum stepper_axis axis, double rate, long duration)
{
	int rc;
	struct stepper_parameters *sp;
	const char *names[2] = { "pimount.ra", "pimount.dec" };
	enum stepper_direction direction;
	pthread_t *thread;

	/* Verify that the Axis is Valid */

	if ((STEPPER_AXIS_RA != axis) && (STEPPER_AXIS_DEC != axis)) {
		fprintf(stderr, "%s:%d - Invalid Axis: %s\n",
			__FILE__, __LINE__, stepper_axis_names(axis));

		return -1;
	}

	/* If the rate is exactly 0.0, stop. */

	if (0.0 == rate)
		return stepper_stop(axis);

	/* Make sure the rate is valid.	*/

	if (fabs(rate - 0.0) < SAME_DOUBLE) {
		fprintf(stderr, "%s:%d - Invalid Rate %f (too close to zero)!\n",
			__FILE__, __LINE__, rate);

		return -1;
	}

	/* Set the Direction */

	if (rate > 0.0)
 		direction = STEPPER_DIRECTION_POSITIVE;
	else
 		direction = STEPPER_DIRECTION_NEGATIVE;

	/* Lock */

	lock(&global.mutex);

	/* Initialize Locals and Check State */

	if (STEPPER_AXIS_RA == axis) {
		sp = &global.ra_parameters;

		if (STEPPER_STATE_OFF != sp->state) {
			fprintf(stderr, "%s:%d - RA is in use!\n",
				__FILE__, __LINE__);
			unlock(&global.mutex);

			return -1;
		}

		thread = &global.ra_thread;
	} else {
		sp = &global.dec_parameters;

		if (STEPPER_STATE_OFF != sp->state) {
			fprintf(stderr, "%s:%d - DEC is in use!\n",
				__FILE__, __LINE__);
			unlock(&global.mutex);

			return -1;
		}

		thread = &global.dec_thread;
	}

	rc = pthread_mutex_lock(&sp->mutex);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_mutex_lock() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));
		unlock(&global.mutex);

		return -1;
	}

	/* Initialize sp using the inputs. */
	sp->rate = rate;
	sp->direction = direction;
	sp->duration = duration;

	if (STEPPER_AXIS_RA == axis) {
		if (ra_update_from_rate(sp)) {
			fprintf(stderr,	"%s:%d - ra_update_from_rate() failed!\n",
				__FILE__, __LINE__);
			pthread_mutex_unlock(&sp->mutex);
			unlock(&global.mutex);

			return -1;
		}
	} else {
		if (dec_update_from_rate(sp)) {
			fprintf(stderr,	"%s:%d - dec_update_from_rate() failed!\n",
				__FILE__, __LINE__);
			pthread_mutex_unlock(&sp->mutex);
			unlock(&global.mutex);

			return -1;
		}
	}

	sp->state = STEPPER_STATE_ON;
	sp->remaining = duration;

	rc = pthread_create(thread, NULL, stepper, (void *)sp);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_create() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));
		pthread_mutex_unlock(&sp->mutex);
		unlock(&global.mutex);

		return -1;
	}

	rc = pthread_setname_np(*thread, names[axis]);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setname_np() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));
		pthread_cancel(*thread);
		pthread_join(*thread, NULL);
		pthread_mutex_unlock(&sp->mutex);
		unlock(&global.mutex);

		return -1;
	}

	pthread_mutex_unlock(&sp->mutex);

	/* Release the Globals Lock */

	unlock(&global.mutex);

	/*
	  Report the Results
	*/

	printf("Started running %s at %.2f arcsec/sec for",
	       (axis == STEPPER_AXIS_RA) ? "RA" : "DEC", rate);

	if (0 == duration)
		printf("ever.\n");
	else
		printf(" %.3f sec.\n", ((double)duration / 1000.0));

	return EXIT_SUCCESS;
}

/*
  ------------------------------------------------------------------------------
  stepper_stop
*/

int
stepper_stop(enum stepper_axis axis)
{
	int rc = 0;
	pthread_t *thread;

	/*
	  Get the Globals Lock
	*/

	lock(&global.mutex);

	if (STEPPER_AXIS_RA == axis) {
		thread = &global.ra_thread;
	} else {
		thread = &global.dec_thread;
	}

	rc = pthread_cancel(*thread);

	if (rc) {
		/* Don't complain if the thread already exited. */
		if (ESRCH != rc)
			fprintf(stderr,
				"pthread_cancel() failed: %s\n", strerror(rc));
	} else {
		pthread_join(*thread, NULL);
	}

	/*
	  Release the Globals Lock
	*/

	unlock(&global.mutex);

	return 0;
}

/*
  ------------------------------------------------------------------------------
  stepper_get_status
*/

int
stepper_get_status(enum stepper_axis axis,
		   bool *running, double *rate, long int *remaining)
{
	struct stepper_parameters *sp;

	if (STEPPER_AXIS_INVALID == axis) {
		fprintf(stderr, "Invalid Axis!\n");

		return -1;
	}

	lock(&global.mutex);

	if (STEPPER_AXIS_RA == axis)
		sp = &global.ra_parameters;
	else
		sp = &global.dec_parameters;

	if (STEPPER_STATE_INVALID == sp->state) {
		unlock(&global.mutex);
		fprintf(stderr, "Invalid State!\n");

		return -1;
	}

	if (NULL != running) {
		if (STEPPER_STATE_ON == sp->state)
			*running = true;
		else
			*running = false;
	}

	if (NULL != rate)
		*rate = sp->rate;

	if (NULL != remaining)
		*remaining = sp->remaining;

	unlock(&global.mutex);

	return 0;
}
