/*
  stepper.h
*/

#ifndef __STEPPER__
#define __STEPPER__

#include <math.h>

enum motion_state { MOTION_STATE_OFF = 0, MOTION_STATE_ON = 1 };
enum motion_axis { MOTION_AXIS_RA = 0, MOTION_AXIS_DEC = 1 };

/*
  Use NW to mean North for dec and West for ra.  SE is South for dec
  and East for ra.
*/

enum motion_direction { MOTION_DIR_NW = 0, MOTION_DIR_SE = 1 };

struct speed {
	enum motion_state state;
	enum a4988_res resolution;
	enum a4988_dir direction;
	unsigned width;		/* micro seconds */
	unsigned delay;		/* micro seconds */
};

struct motion {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	struct a4988 driver;
	struct speed *speed;
};

/*
  Convert rate (in arc seconds / second) into the necessary a4988
  parameters.  The axis is required because the calculation is
  different for RA and DEC.

  The Earth's rotation rate is about 15 arcseconds per second.
*/

int stepper_set_rate(struct speed *speed,
		     enum motion_state state,
		     enum motion_axis axis,
		     enum motion_direction direction,
		     double rate); /* rate is in arcsecs/sec */

/*
  Convert the speed struction to arc seconds per second.
*/

int stepper_get_rate(struct speed *speed, enum motion_axis axis, double *rate);

/*
  The main stepper thread.  Should be started with pthread_create()
  and given a struct motion pointer as input.
*/

void *stepper(void *);

#endif	/* __STEPPER__ */
