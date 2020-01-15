/*
  output.c

  Test the output...
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

#include <pigpio.h>

#include "../timespec.h"
#include "../a4988.h"

char *cmdErrStr(int);		/* For some reason, pigpio doesn't export this! */

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("output <pins> <resolution> <direction> <width> <delay> <steps>\n" \
	       "<pins> : A ':' separated list of the gpio pins (5)\n" \
	       "    pin connected to direction\n" \
	       "    pin connected to step\n" \
	       "    pin connected to sleep\n" \
	       "    pin connected to ms2\n" \
	       "    pin connected to ms1\n" \
	       "<resolution> : 0:FULL, 1:HALF, 2:QUARTER, 3:EIGHTH\n" \
	       "<direction> : 0:CW, 1:CCW\n" \
	       "<width> : Width of the pulse in micro seconds\n" \
	       "<delay> : Delay between pulses in micro seconds\n" \
	       "<steps> : Number of steps, 0 means forever...\n");

	exit(exit_code);
}

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	printf("--> Terminated...\n");
	gpioTerminate();
	exit(EXIT_FAILURE);
}

/*
  ------------------------------------------------------------------------------
  drive
*/

static int
drive(int pins[], enum a4988_res resolution, enum a4988_dir direction,
      unsigned long long width, unsigned long long _delay, unsigned steps)
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
	period.tv_nsec = (width * 1000) + (_delay * 1000);
	period = timespec_normalise(period);
	delay.tv_sec = 0;
	delay.tv_nsec = (_delay * 1000);
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

		rc = a4988_step(&driver, width);

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
	char *token;
	unsigned i;
	int pins[5];
	int resolution = -1;
	int direction = -1;
	int width_given = 0;
	unsigned long long width;
	int delay_given = 0;
	unsigned long long delay;
	int steps = -1;

	static struct option long_options[] = {
		{"help",       no_argument,       0,  'h' },
		{"pins",       required_argument, 0,  'p' },
		{"resolution", required_argument, 0,  'r' },
		{"direction",  required_argument, 0,  'd' },
		{"width",      required_argument, 0,  'w' },
		{"delay",      required_argument, 0,  'e' },
		{"steps",      required_argument, 0,  's' },
		{0, 0, 0, 0}
	};

	for (i = 0; i < (sizeof(pins) / sizeof(int)); ++i)
		pins[i] = -1;

	while ((opt = getopt_long(argc, argv, "hp:r:d:w:e:s:", 
				  long_options, &long_index )) != -1) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'p':
			token = strtok(optarg, ":");
			i = 0;

			while (NULL != token) {
				pins[i++] = atoi(token);
				token = strtok(NULL, ":");
			}

			break;
		case 'r':
			resolution = atoi(optarg);

			if (A4988_RES_FULL != resolution &&
			    A4988_RES_HALF != resolution &&
			    A4988_RES_QUARTER != resolution &&
			    A4988_RES_EIGHTH != resolution) {
				printf("Bad Resolution: %d\n", resolution);

				return EXIT_FAILURE;
			}

			break;
		case 'd':
			direction = atoi(optarg);
	
			if (A4988_DIR_CW != direction &&
			    A4988_DIR_CCW != direction) {
				printf("Bad Direction: %d\n", direction);

				return EXIT_FAILURE;
			}

			break;
		case 'w':
			width_given = 1;
			width = strtoull(optarg, NULL, 0);
			break;
		case 'e':
			delay_given = 1;
			delay = strtoull(optarg, NULL, 0);
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

	if (resolution == -1 ||
	    direction == -1 ||
	    !width_given ||
	    !delay_given ||
	    steps == -1)
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
