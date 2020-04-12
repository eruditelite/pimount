/*
  rate.c

  Run at a specified rate using the stepper threads.
*/

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

#include <pigpio.h>

#include "../timespec.h"
#include "../a4988.h"
#include "../stepper.h"

char *cmdErrStr(int);		/* For some reason, pigpio doesn't export this! */

static struct speed speed;
static struct motion motion;
static pthread_t stepper_thread;

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("rate -p <pins> -a <axis> -d <direction> -r <rate>\n" \
	       "<pins> : A ':' separated list of the gpio pins (5)\n" \
	       "    pin connected to direction\n" \
	       "    pin connected to step\n" \
	       "    pin connected to sleep\n" \
	       "    pin connected to ms2\n" \
	       "    pin connected to ms1\n" \
	       "<axis> : 0:ra or 1:dec\n" \
	       "<direction> : 0:N|W 1:S|E\n" \
	       "<rate> : Rate in arc seconds per second\n");

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
	pthread_cancel(stepper_thread);
	pthread_join(stepper_thread, NULL);
	gpioTerminate();

	pthread_exit(NULL);
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
	int axis = -1;
	int direction = -1;
	double rate = -1.0;
	char name_buf[DESCRIPTION_SIZE];

	static struct option long_options[] = {
		{"help",       no_argument,       0,  'h' },
		{"pins",       required_argument, 0,  'p' },
		{"axis",       required_argument, 0,  'a' },
		{"direction",  required_argument, 0,  'd' },
		{"rate",       required_argument, 0,  'r' },
		{0, 0, 0, 0}
	};

	for (i = 0; i < (sizeof(pins) / sizeof(int)); ++i)
		pins[i] = -1;

	while ((opt = getopt_long(argc, argv, "hp:a:d:r:", 
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
		case 'a':
			axis = atoi(optarg);

			if (MOTION_AXIS_RA != axis &&
			    MOTION_AXIS_DEC != axis) {
				printf("Bad Axis: %d\n", axis);

				return EXIT_FAILURE;
			}

			break;
		case 'd':
			direction = atoi(optarg);
	
			if (MOTION_DIR_NW != direction &&
			    MOTION_DIR_SE != direction) {
				printf("Bad Direction: %d\n", direction);

				return EXIT_FAILURE;
			}

			break;
		case 'r':
			rate = atof(optarg);
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

	if (axis == -1 || direction == -1 || rate == -1.0)
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
	  Start the Stepper Thread
	*/

	memset(name_buf, 0, sizeof(name_buf));
	sprintf(name_buf, "%s Stepper Test", axis == 0 ? "RA" : "DEC" );
	strcpy(motion.driver.description, name_buf);
	motion.driver.direction = pins[0];
	motion.driver.step = pins[1];
	motion.driver.sleep = pins[2];
	motion.driver.ms2 = pins[3];
	motion.driver.ms1 = pins[4];
	motion.speed = &speed;

	if (EXIT_SUCCESS != stepper_set_rate(motion.speed,
					     MOTION_STATE_ON,
					     axis, direction, rate)) {
		fprintf(stderr, "stepper_set_speed() failed!\n");

		return EXIT_FAILURE;
	}

	pthread_mutex_init(&motion.mutex, NULL);
	pthread_mutex_lock(&motion.mutex);
	pthread_cond_init(&motion.cond, NULL);

	if (0 != a4988_initialize(&(motion.driver))) {
		fprintf(stderr, "DEC Initialization Failed!\n");
		a4988_finalize(&(motion.driver));
		gpioTerminate();

		return EXIT_FAILURE;
	} else {
		rc = pthread_create(&stepper_thread, NULL, stepper,
				    (void *)&motion);

		if (0 != rc) {
			fprintf(stderr, "pthread_create() failed: %d\n", rc);

			return EXIT_FAILURE;
		}

		rc = pthread_setname_np(stepper_thread, name_buf);

		if (0 != rc)
			fprintf(stderr,
				"pthread_setname_np() failed: %d\n", rc);
	}

	pthread_mutex_unlock(&motion.mutex);

	/*
	  Wait for a Signal
	*/

	for (;;)
		sleep(10);

	/*
	  Clean Up
	*/

	pthread_cancel(stepper_thread);
	pthread_join(stepper_thread, NULL);
	gpioTerminate();

	return EXIT_SUCCESS;
}
