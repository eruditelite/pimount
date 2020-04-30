/*
  ==============================================================================
  output.c

  Test a4988.*.
  ==============================================================================
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

#include <pigpio.h>

#include "../pimount.h"
#include "../timespec.h"
#include "../a4988.h"

char *cmdErrStr(int); /* For some reason, pigpio doesn't export this! */

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

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("output \n"
	       "--axis|-a, Axis to driver, ra|dec\n"
	       "--resolution|-r, Stepper Resolution : full|half|quarter|eighth\n"
	       "--direction|-d, Stepper Direction : cw|ccw\n"
	       "--width|-w, Width of the pulse in micro seconds.\n"
	       "--delay|-e, Delay between pulses in micro seconds.\n"
	       "--steps|-s, Number of steps, 0 means forever...\n");

	exit(exit_code);
}

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	printf("--> Ouput Test Terminated...\n");
	gpioTerminate();
	exit(EXIT_FAILURE);
}

/*
  ------------------------------------------------------------------------------
  drive
*/

static int
drive(int pins[], enum a4988_res resolution, enum a4988_dir direction,
      unsigned widthin, unsigned delayin, unsigned steps)
{
	int rc;
	struct a4988 driver;
	struct timespec period;
	struct timespec delay;
	struct timespec sleep;
	struct timespec adjust[2];
	int ai = 0;

	memset(&driver, 0, sizeof(driver));

	driver.direction = pins[0];
	driver.step = pins[1];
	driver.sleep = pins[2];
	driver.ms2 = pins[3];
	driver.ms1 = pins[4];

	rc = a4988_initialize(&driver);

	if (0 != rc) {
		printf("a4988_initialize() failed!\n");

		return EXIT_FAILURE;
	}

	rc = a4988_enable(&driver, resolution, direction);

	if (0 != rc) {
		printf("a4988_enable() failed!\n");

		return EXIT_FAILURE;
	}

	period.tv_sec = 0;
	period.tv_nsec = (widthin * 1000) + (delayin * 1000);
	period = timespec_normalise(period);
	delay.tv_sec = 0;
	delay.tv_nsec = (delayin * 1000);
	delay = timespec_normalise(delay);
	sleep = delay;

	for (;;) {
		struct timespec offset;

		clock_gettime(CLOCK_MONOTONIC, &adjust[ai++]);

		if (2 == ai) {
			offset = timespec_sub(adjust[1], adjust[0]);
			offset = timespec_normalise(offset);

			if (timespec_gt(period, offset)) {
				offset = timespec_sub(period, offset);
				sleep = timespec_add(sleep, offset);
			} else {
				offset = timespec_sub(offset, period);
				sleep = timespec_sub(sleep, offset);
			}

			ai = 0;
		}

		rc = a4988_step(&driver, widthin);

		if (0 != rc) {
			printf("a4988_step() failed!\n");

			return EXIT_FAILURE;
		}

		nanosleep(&sleep, NULL);

		if (0 < steps) {
			--steps;

			if (0 == steps)
				break;
		}
	}

	rc = a4988_disable(&driver);

	if (0 != rc) {
		printf("a4988_disable() failed!\n");

		return EXIT_FAILURE;
	}

	a4988_finalize(&driver);

	return EXIT_SUCCESS;
}

/*
  ------------------------------------------------------------------------------
  main
*/

int
main(int argc, char *argv[])
{
	int rc;
	int opt = 0;
	int long_index = 0;
	unsigned i;
	int pins[5];
	enum a4988_res resolution = A4988_RES_INVALID;
	enum a4988_dir direction = A4988_DIR_INVALID;
	int width_given = 0;
	unsigned width = 0;
	int delay_given = 0;
	unsigned delay = 0;
	int steps = -1;

	static struct option long_options[] = {
		{"help",       no_argument,       0,  'h' },
		{"axis",       required_argument, 0,  'a' },
		{"resolution", required_argument, 0,  'r' },
		{"direction",  required_argument, 0,  'd' },
		{"width",      required_argument, 0,  'w' },
		{"delay",      required_argument, 0,  'e' },
		{"steps",      required_argument, 0,  's' },
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "ha:r:d:w:e:s:", 
				  long_options, &long_index )) != -1) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'a':
			if (0 == strncmp(optarg, "ra", strlen("ra"))) {
				pins[0] = RA_PIN_DIRECTION;
				pins[1] = RA_PIN_STEP;
				pins[2] = RA_PIN_SLEEP;
				pins[3] = RA_PIN_MS2;
				pins[4] = RA_PIN_MS1;
			} else if (0 == strncmp(optarg, "dec", strlen("dec"))) {
				pins[0] = DEC_PIN_DIRECTION;
				pins[1] = DEC_PIN_STEP;
				pins[2] = DEC_PIN_SLEEP;
				pins[3] = DEC_PIN_MS2;
				pins[4] = DEC_PIN_MS1;
			} else {
				fprintf(stderr, "Invalid Axis!\n");
				usage(EXIT_FAILURE);
			}

			break;
		case 'r':
			if (0 == strncmp(optarg, "full",
					 strlen("full"))) {
				resolution = A4988_RES_FULL;
			} else if (0 == strncmp(optarg, "half",
						strlen("half"))) {
				resolution = A4988_RES_HALF;
			} else if (0 == strncmp(optarg, "quarter",
						strlen("quarter"))) {
				resolution = A4988_RES_QUARTER;
			} else if (0 == strncmp(optarg, "eighth",
						strlen("eighth"))) {
				resolution = A4988_RES_EIGHTH;
			} else {
				fprintf(stderr, "Invalid Resolution!\n");
				usage(EXIT_FAILURE);
			}

			break;
		case 'd':
			if (0 == strncmp(optarg, "cw", strlen("cw"))) {
				direction = A4988_DIR_CW;
			} else if (0 == strncmp(optarg, "ccw", strlen("ccw"))) {
				direction = A4988_DIR_CCW;
			} else {
				fprintf(stderr, "Invalid Direction!\n");
				usage(EXIT_FAILURE);
			}

			break;
		case 'w':
			width_given = 1;
			width = strtoul(optarg, NULL, 0);
			break;
		case 'e':
			delay_given = 1;
			delay = strtoul(optarg, NULL, 0);
			break;
		case 's':
			steps = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Invalid Option\n");
			usage(EXIT_FAILURE);
			break;
		}
	}

	/* Make sure the Pins Got Set */

	for (i = 0; i < (sizeof(pins) / sizeof(int)); ++i) {
		if (-1 == pins[i]) {
			fprintf(stderr, "Invalid GPIO Pin\n");

			return EXIT_FAILURE;
		}
	}

	/* Make Sure Other Arguments Got Set */

	if (resolution == A4988_RES_INVALID ||
	    direction == A4988_DIR_INVALID ||
	    !width_given || !delay_given || steps == -1)
		usage(EXIT_SUCCESS);

	/*
	  Initialize pigpio
	*/

	rc = gpioInitialise();

	if (PI_INIT_FAILED == rc) {
		fprintf(stderr, "gpioinitialise() failed: %s\n", cmdErrStr(rc));

		return EXIT_FAILURE;
	}

 	/*
	  Catch Signals
	*/

	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGCONT, handler);
	signal(SIGTERM, handler);

	/*
	  Run a Test
	*/

	rc = drive(pins, resolution, direction, width, delay, steps);

	if (rc == 0)
		printf("--> Glorious Success!!\n");
	else
		printf("--> Abject Failure...\n");

	/*
	  Clean Up
	*/

	gpioTerminate();

	return EXIT_SUCCESS;
}
