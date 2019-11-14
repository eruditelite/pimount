/*
  a4988.h

  Notes
  =====

  -1-
  After calling a4988_enable(), current is flowing.  Calling
  a4988_disable() will put the controller in sleep mode (no current
  flowing).

  Design Decisions
  ================

  -1-
   As this is meant to be used with a very specific hardware setup,
   the original mode of the GPIO pins is not saved/restored.

  -2-
  The A4988 offers a /16 microstep function.  In any of my use cases,
  /8 is quite adequate!  /16 would require one more pin, so it is
  ignored.

  -3-
  The finalize function returns void. What would you do if it failed?
*/

#ifndef _A4988_H_
#define _A4988_H_

#define DESCRIPTION_SIZE 80

struct a4988 {
	/* 0 means sleep -- allow 1 ms before stepping after setting to 1. */
	unsigned sleep;

	unsigned direction;

	/* At least 1 ns. */
	unsigned step;

	/*
	      Microstep
	      Resolution MS1 MS2 MS3

	       Full Step 0   0   0
	       Half Step 1   0   0
	     Quater Step 0   1   0
	     Eighth Step 1   1   0
	  Sixteenth Step 1   1   1 (unused)
	*/

	unsigned ms1;
	unsigned ms2;

	unsigned position;

	char description[DESCRIPTION_SIZE];
};

enum a4988_res {
	A4988_RES_FULL = 0,
	A4988_RES_HALF,
	A4988_RES_QUARTER,
	A4988_RES_EIGHTH
};

static inline const char *
a4988_res_names(enum a4988_res resolution)
{
	switch (resolution) {
	case A4988_RES_FULL: return "A4988_RES_FULL"; break;
	case A4988_RES_HALF: return "A4988_RES_HALF"; break;
	case A4988_RES_QUARTER: return "A4988_RES_QUARTER"; break;
	case A4988_RES_EIGHTH: return "A4988_RES_EIGHTH"; break;
	default: break;
	}

	return "BAD RESOLUTION";
};

enum a4988_dir {
	A4988_DIR_CW,
	A4988_DIR_CCW
};

static inline const char *
a4988_dir_names(enum a4988_dir direction)
{
	switch (direction) {
	case A4988_DIR_CW: return "A4988_DIR_CW"; break;
	case A4988_DIR_CCW: return "A4988_DIR_CCW"; break;
	default: break;
	}

	return "BAD DIRECTION";
}

int a4988_initialize(struct a4988 *);
void a4988_finalize(struct a4988 *);
int a4988_enable(struct a4988 *, enum a4988_res, enum a4988_dir);
int a4988_disable(struct a4988 *);
int a4988_step(struct a4988 *);

#endif	/* _A4988_H_ */
