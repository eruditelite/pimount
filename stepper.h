/*
  ==============================================================================
  stepper.h

  API for RA and DEC steppers.
  ==============================================================================
*/

#ifndef __STEPPER__
#define __STEPPER__

#include <math.h>

enum stepper_state {
	STEPPER_STATE_INVALID = -1,
	STEPPER_STATE_OFF = 0,	/* Turn the A4988 Off */
	STEPPER_STATE_ON = 1	/* Turn the A4988 On */
};

__attribute__ ((unused)) static const char *
stepper_state_names(enum stepper_state state)
{
	switch (state) {
	case STEPPER_STATE_INVALID:
		return "STEPPER_STATE_INVALID"; break;
	case STEPPER_STATE_OFF:
		return "STEPPER_STATE_OFF"; break;
	case STEPPER_STATE_ON:
		return "STEPPER_STATE_ON"; break;
	default: break;
	}

	return "BAD STATE";
}

enum stepper_axis {
	STEPPER_AXIS_INVALID = -1,
	STEPPER_AXIS_RA = 0,
	STEPPER_AXIS_DEC = 1
};

__attribute__ ((unused)) static const char *
stepper_axis_names(enum stepper_axis axis)
{
	switch (axis) {
	case STEPPER_AXIS_INVALID:
		return "STEPPER_AXIS_INVALID"; break;
	case STEPPER_AXIS_RA:
		return "STEPPER_AXIS_RA"; break;
	case STEPPER_AXIS_DEC:
		return "STEPPER_AXIS_DEC"; break;
	default: break;
	}

	return "BAD AXIS";
}

/*
  Use positive, STEPPER_DIRECTION_POS, to mean West in the RA case and
  North in the DEC case.  STEPPER_DIRECTION_NEG means the opposite...
*/

enum stepper_direction {
	STEPPER_DIRECTION_INVALID = -1,
	STEPPER_DIRECTION_POSITIVE = 0,
	STEPPER_DIRECTION_NEGATIVE = 1
};

__attribute__ ((unused)) static const char *
stepper_direction_names(enum stepper_direction direction)
{
	switch (direction) {
 	case STEPPER_DIRECTION_INVALID:
		return "STEPPER_DIRECTION_INVALID"; break;
 	case STEPPER_DIRECTION_POSITIVE:
		return "STEPPER_DIRECTION_POSITIVE"; break;
	case STEPPER_DIRECTION_NEGATIVE:
		return "STEPPER_DIRECTION_NEGATIVE"; break;
	default: break;
	}

	return "BAD DIRECTION";
}

int stepper_initialize(void);
void stepper_finalize(void);

/*
  rate is in arcseconds per second (15 arcseconds per second it tracking)

  if rate is postive, direction is STEPPER_DIRECTION_POSITIVE
  if rate is negative, direction is STEPPER_DIRECTION_NEGATIVE

  duration is in milli seconds

  if duration is 0, run until stopped
*/

int stepper_start(enum stepper_axis axis, double rate, long duration);

int stepper_stop(enum stepper_axis axis);

int stepper_get_status(enum stepper_axis axis,
		       bool *running, double *rate, long int *remaining);

#endif	/* __STEPPER__ */
