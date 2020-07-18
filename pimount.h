/*
  pimount.h
*/

#ifndef __PIMOUNT__H
#define __PIMOUNT__H

#include <pthread.h>

#define JOYSTICK "/dev/input/js0"

/*
  Choose a pin that supports PWM, which for Raspberry Pi means 18.
*/

#define FAN_PIN 18

#define RA_PIN_DIRECTION 26
#define RA_PIN_STEP 19
#define RA_PIN_SLEEP 13
#define RA_PIN_MS2 6
#define RA_PIN_MS1 5

#define DEC_PIN_DIRECTION 27
#define DEC_PIN_STEP 17
#define DEC_PIN_SLEEP 4
#define DEC_PIN_MS2 3
#define DEC_PIN_MS1 2

enum pimount_control {
	PIMOUNT_CONTROL_OFF,
	PIMOUNT_CONTROL_LOCAL,
	PIMOUNT_CONTROL_REMOTE
};

__attribute__ ((unused)) static const char *
pimount_control_names(enum pimount_control control)
{
	switch (control) {
	case PIMOUNT_CONTROL_OFF:
		return "PIMOUNT_CONTROL_OFF"; break;
	case PIMOUNT_CONTROL_LOCAL:
		return "PIMOUNT_CONTROL_LOCAL"; break;
	case PIMOUNT_CONTROL_REMOTE:
		return "PIMOUNT_CONTROL_REMOTE"; break;
	default: break;
	}

	return "BAD STATE";
}

#define MAX_RA_RATE 120.0
#define MAX_DEC_RATE 120.0

struct pimount_state {
	pthread_mutex_t mutex;
	enum pimount_control control;
	double ra_rate;
	double dec_rate;
};

extern struct pimount_state state;

/*
  (Un)Locking

  If acquiring or releasing a lock every fails, die horribly.
*/

void _lock(pthread_mutex_t *mutex, const char *file, int line);
#define lock(mutex) _lock(mutex, __FILE__, __LINE__);
void _unlock(pthread_mutex_t *mutex, const char *file, int line);
#define unlock(mutex) _unlock(mutex, __FILE__, __LINE__);

#endif	/* __PIMOUNT__H */
