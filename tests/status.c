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
#include "../pimount.h"

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

	/* Clear the screen, and print the labels. */
	oled_clear(i2c_handle);
	oled_print(i2c_handle, 0, 0, OLED_FONT_MEDIUM, "PiMount");
	oled_print(i2c_handle, 0, 2, OLED_FONT_MEDIUM, "T/L");
	oled_print(i2c_handle, 0, 4, OLED_FONT_MEDIUM, "R/A");
	oled_print(i2c_handle, 0, 6, OLED_FONT_MEDIUM, "DEC");
	sleep(1);

	for (;;) {
		int temp;
		long load;
		struct pimount_state state_copy;
		char buffer[80];
		int flen;

		/* Clear the value areas (after the labels above). */
		oled_fill(i2c_handle, false, 3, 2, 15, 2);
		oled_fill(i2c_handle, false, 3, 4, 15, 6);

		/* Update State */

		lock(&state.mutex);
		state_copy.control = state.control;
		state_copy.ra_rate = -60.0; /*state.ra_rate;*/
  		state_copy.dec_rate = 33.2; /*state.dec_rate;*/
		unlock(&state.mutex);

		/* Update 'control' */

		flen = 15 - 7;	/* Available Space */

		switch (state_copy.control) {
		case PIMOUNT_CONTROL_OFF:
			snprintf(buffer, flen, "Parked");
			break;
		case PIMOUNT_CONTROL_LOCAL:
			snprintf(buffer, flen, "Local");
			break;
		case PIMOUNT_CONTROL_REMOTE:
			snprintf(buffer, flen, "Remote");
			break;
		default:
			break;
		}

		oled_fill(i2c_handle, false, 7, 0, 15, 0);
		oled_print(i2c_handle, 15 - strlen(buffer), 0, OLED_FONT_MEDIUM,
			   buffer);

		flen = 15 - 3;	/* Available space for ra/dec rates. */
		snprintf(buffer, flen, "%.2f", state_copy.ra_rate);
		oled_fill(i2c_handle, false, 3, 4, 15, 4);
		oled_print(i2c_handle, 15 - strlen(buffer), 4, OLED_FONT_MEDIUM,
			   buffer);
		snprintf(buffer, flen, "%.2f", state_copy.dec_rate);
		oled_fill(i2c_handle, false, 3, 6, 15, 6);
		oled_print(i2c_handle, 15 - strlen(buffer), 6, OLED_FONT_MEDIUM,
			   buffer);

		/* Update Temperature and Load */

		load = get_load();
		temp = get_temp();

		if (-1 == load || -1 == temp) {
			sleep(1);
			continue;
		}

		flen = 15 - 3;	/* Available space for T/L. */
		snprintf(buffer, flen, "%dC/%ld%%", temp, load);
		oled_fill(i2c_handle, false, 3, 2, 15, 2);
		oled_print(i2c_handle, 15 - strlen(buffer), 2, OLED_FONT_MEDIUM,
			   buffer);

		/* Sleep for a Bit */

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
