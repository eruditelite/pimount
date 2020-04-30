/*
  ==============================================================================
  status.c

  Test the OLED local status display.
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
#include <stdbool.h>

#include <pigpio.h>

#include "../oled.h"
#include "../stats.h"

char *cmdErrStr(int); /* For some reason, pigpio doesn't export this! */

static int i2c_handle = -1;

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("status \n"
	       "--help|-h, Display this wonderful help screen...\n");

	exit(exit_code);
}

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	printf("--> Status Test Terminated...\n");

	if (0 <= i2c_handle) {
		oled_finalize(i2c_handle);
		i2cClose(i2c_handle);
	}

	gpioTerminate();

	exit(EXIT_FAILURE);
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

	static struct option long_options[] = {
		{"help",       no_argument,       0,  'h' },
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "h", 
				  long_options, &long_index )) != -1) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			fprintf(stderr, "Invalid Option\n");
			usage(EXIT_FAILURE);
			break;
		}
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
	  Set Up I2C Connection
	*/

	i2c_handle = i2cOpen(1, 0x3c, 0);

	if (0 > i2c_handle) {
		fprintf(stderr, "i2cOpen() failed: %s\n", cmdErrStr(rc));
		gpioTerminate();

		return EXIT_FAILURE;
	}

	/*
	  Initialize the Display
	*/

	rc = oled_initialize(i2c_handle, false, false);

	if (rc) {
		fprintf(stderr,
			"%s:%d - oled_initialize() failed\n", __FILE__, __LINE__);
		gpioTerminate();

		return EXIT_FAILURE;
	}

	rc = stats_initialize();

	if (rc) {
		fprintf(stderr,
			"%s:%d - stats_initialize() failed\n",
			__FILE__, __LINE__);
		oled_finalize(i2c_handle);
		gpioTerminate();

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
	  Do Something
	*/

	oled_clear(i2c_handle);
	sleep(1);

	for (;;) {
		int temp;
		long load;
		char buffer[80];

		load = get_load();
		temp = get_temp();

		if (-1 == load || -1 == temp) {
			sleep(1);
			continue;
		}

		oled_clear(i2c_handle);
		oled_print(i2c_handle, 0, 0, OLED_FONT_MEDIUM,
			   "PiMount   Parked");
		sprintf(buffer, "T/L %dC/%ld%%", temp, load);
		oled_print(i2c_handle, 0, 2, OLED_FONT_MEDIUM, buffer);
		sleep(5);
	}

	pause();

	/*
	  Clean Up
	*/

	stats_finalize();

	oled_finalize(i2c_handle);

	if (0 <= i2c_handle)
		i2cClose(i2c_handle);

	gpioTerminate();

	return EXIT_SUCCESS;
}
