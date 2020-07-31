/*
  ==============================================================================
  ==============================================================================
  rate.c

  Run at a specified rate using one of the stepper threads.
  ==============================================================================
  ==============================================================================
*/

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
#include <sched.h>

#include <pigpio.h>

#include "../timespec.h"
#include "../a4988.h"
#include "../stepper.h"

char *cmdErrStr(int); /* For some reason, pigpio doesn't export this! */

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("rate \n"
	       "--axis|-a, Axis to driver, ra|dec\n"
	       "--duration|-d, Run time in milli seconds (0 means forever).\n"
	       "--rate|-r, Rate in arc seconds per second.\n");

	exit(exit_code);
}

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	printf("--> Rate Test Terminated...\n");
	stepper_finalize();
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
	enum stepper_axis axis = STEPPER_AXIS_INVALID;
 	double rate = -1.0;
	long duration = -1;

	static struct option long_options[] = {
		{"help",      no_argument,       0,  'h' },
		{"axis",      required_argument, 0,  'a' },
		{"duration",  required_argument, 0,  'd' },
		{"rate",      required_argument, 0,  'r' },
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "ha:d:r:", 
				  long_options, &long_index )) != -1) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'a':
			if (0 == strncmp(optarg, "ra", strlen("ra"))) {
				axis = STEPPER_AXIS_RA;
			} else if (0 == strncmp(optarg, "dec", strlen("dec"))) {
				axis = STEPPER_AXIS_DEC;
			} else {
				fprintf(stderr, "Invalid Axis!\n");
				usage(EXIT_FAILURE);
			}

			break;
		case 'd':
			duration = atoi(optarg);
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

	/*
	  Make Sure Other Arguments Got Set
	*/

	if (STEPPER_AXIS_INVALID == axis ||
	    duration == -1.0 || rate == -1.0)
		usage(EXIT_FAILURE);

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
	  Start Stepping
	*/

	rc = stepper_initialize();

	if (rc) {
		gpioTerminate();
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);

		return EXIT_FAILURE;
	}

	rc = stepper_start(axis, rate, duration);

	if (rc) {
		stepper_finalize();
		gpioTerminate();
		fprintf(stderr, "%s:%d - rc=%d\n",
			__FILE__, __LINE__, rc);

		return EXIT_FAILURE;
	}

	if (0 == duration)
		/* 0 means forever, just wait for Ctrl-C */
		pause();
	else
		/* Sleep for Duration, and then... */
		usleep(duration * 1000);

	/* wait for the Stepper to Complete */

	for (;;) {
		bool running;
		double rate;
		long int remaining;

		sched_yield();

		if (stepper_get_status(axis, &running, &rate, &remaining))
			fprintf(stderr, "stepper_get_status() failed!\n");

		if (0 == remaining)
			break;
	}

	stepper_stop(axis);

	/*
	  Clean Up
	*/

	stepper_finalize();
	gpioTerminate();

	return EXIT_SUCCESS;
}
