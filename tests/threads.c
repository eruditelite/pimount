/*
  ==============================================================================
  ==============================================================================
  threads.c

  Start and stop the stepper...
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
#include <unistd.h>

#include <pigpio.h>

#include "../pimount.h"
#include "../timespec.h"
#include "../a4988.h"
#include "../stepper.h"

char *cmdErrStr(int); /* For some reason, pigpio doesn't export this! */

/*
  ------------------------------------------------------------------------------
  test_case_1
*/

static int
test_case_1(enum stepper_axis axis)
{
	int rc;

	printf("--> First Test Case\n"
	       "\t-Start a Tracking Thread (Infinite Duration)\n"
	       "\t-Stop it After a Few Seconds\n");

	rc = stepper_start(axis, 15.0, 0);

 	if (rc) {
		fprintf(stderr,
			"%s:%d - stepper_start() failed: %d\n",
			__FILE__, __LINE__, rc);

		return -1;
	}

	sleep(1);

	rc = stepper_stop(axis);

 	if (rc) {
		fprintf(stderr,
			"%s:%d - stepper_stop() failed: %d\n",
			__FILE__, __LINE__, rc);

		return -1;
	}

	return 0;
}

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("threads \n"
	       "--axis|-a, Axis to driver, ra|dec\n"
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
	printf("--> Threads Test Terminated...\n");
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

	static struct option long_options[] = {
		{"help",      no_argument,       0,  'h' },
		{"axis",      required_argument, 0,  'a' },
		{"rate",      required_argument, 0,  'r' },
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "ha:r:",
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
	  Check Parameters
	*/

	if (STEPPER_AXIS_INVALID == axis || (fabs(rate - -1.0) < 0.01)) {
		fprintf(stderr, "Parameter Not Set\n");
		usage(EXIT_FAILURE);
	}

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

	/*
	  Run Test Case 1
	*/

	rc = test_case_1(axis);

	if (rc)
		printf("Test Case 1 Failed!\n");
	else
		printf("Test Case 1 Succeeded!\n");

	sleep(2);
	rc = test_case_1(axis);

	if (rc)
		printf("Test Case 1 Failed!\n");
	else
		printf("Test Case 1 Succeeded!\n");

	/*
	  Clean Up
	*/

	stepper_finalize();
	gpioTerminate();

	return EXIT_SUCCESS;
}
