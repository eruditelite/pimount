/*
  main.c

  == Defaults...

  buttons : 11:27:9:10:22
  ra : 2:3:4:14:15
  dec : 5:6:13:19:26

  == Some results for RA (pins are 2:3:4:14:15)

  One HA Fast

  [ pi@raspberrypi ] time ./simple -s 31250 -p 0 -d cw -r f 2:3:4:14:15
  real	1m17.035s
  user	0m0.302s
  sys	0m3.898s

  One HA in an Hour

  [ pi@raspberrypi ] time ./simple -s 250000 -p 13800 -d ccw -r e 2:3:4:14:15
  real	59m46.303s
  user	0m2.084s
  sys	0m40.113s

  == Some results for Dec (pins are 5:6:13:19:26)
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <pigpiod_if2.h>

#include "pins.h"
#include "a4988.h"

static int pi = -1;

static struct a4988 ra_driver;
static struct a4988 dec_driver;

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	/* Try to shut everything down. */

	a4988_finalize(&ra_driver);
	a4988_finalize(&dec_driver);

	if (-1 != pi)
		pigpio_stop(pi);

	exit(EXIT_FAILURE);
}

/*
  ------------------------------------------------------------------------------
  pb_callback
*/

static void
pb_callback(int pi, unsigned int pin, unsigned int level, unsigned int tick)
{
	printf("%s:%d - pi=%d pin=%u level=%u tick=%u\n",
	       __FILE__, __LINE__,
	       pi, pin, level, tick);

	return;
}

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("pimount <button pins> <ra pins> <da pins>\n" \
	       "<button pins> : A ':' separated list of gpio pins (5)\n" \
	       "         pin connected to +dec\n" \
	       "         pin connected to -dec\n" \
	       "         pin connected to +ra\n" \
	       "         pin connected to -ra\n" \
	       "         pin connected to stop\n" \
	       "<ra|da pins> : A ':' separated list of the gpio pins (5)\n" \
	       "         pin connected to direction\n" \
	       "         pin connected to step\n" \
	       "         pin connected to sleep\n" \
	       "         pin connected to ms2\n" \
	       "         pin connected to ms1\n");

	exit(exit_code);
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
	char ip_address[NAME_MAX];
	char ip_port[NAME_MAX];
	char *token;
	unsigned i;
	unsigned j;
	int pins[3][5];
	int callback_id;

	static struct option long_options[] = {
		{"help",       required_argument, 0,  'h' },
		{0,            0,                 0,  0   }
	};

	strcpy(ip_address, "localhost");
	strcpy(ip_port, "8888");

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

	if (3 != (argc - optind)) {
		fprintf(stderr, "GPIO Pins Must Be Specified\n");
		usage(EXIT_FAILURE);
	}

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < (sizeof(pins[i]) / sizeof(int)); ++j)
			pins[i][j] = -1;

		j = 0;
		token = strtok(argv[optind++], ":");

		while (NULL != token) {
			pins[i][j++] = atoi(token);
			token = strtok(NULL, ":");
		}

		for (j = 0; j < (sizeof(pins[i]) / sizeof(int)); ++j) {
			if (-1 == pins[i][j]) {
				fprintf(stderr, "Invalid GPIO Pin\n");
				return EXIT_FAILURE;
			}

			if ((0 == pins[i][j]) || (1 == pins[i][j])) {
				fprintf(stderr,
					"pigpiod_if2 doesn't allow 0 or 1!\n");
				return EXIT_FAILURE;
			}
		}
	}

	pi = pigpio_start(ip_address, ip_port);

	if (0 > pi) {
		fprintf(stderr, "pigpio_start() failed: %d\n", pi);

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
	  Initialize the Stepper Motor Drivers
	*/

	ra_driver.pigpio = pi;
	ra_driver.direction = pins[1][0];
	ra_driver.step = pins[1][1];
	ra_driver.sleep = pins[1][2];
	ra_driver.ms2 = pins[1][3];
	ra_driver.ms1 = pins[1][4];

	if (0 != a4988_initialize(&ra_driver)) {
		fprintf(stderr, "RA Initialization Failed!\n");
		pigpio_stop(pi);

		return EXIT_FAILURE;
	}

	dec_driver.pigpio = pi;
	dec_driver.direction = pins[2][0];
	dec_driver.step = pins[2][1];
	dec_driver.sleep = pins[2][2];
	dec_driver.ms2 = pins[2][3];
	dec_driver.ms1 = pins[2][4];

	if (0 != a4988_initialize(&dec_driver)) {
		fprintf(stderr, "DEC Initialization Failed!\n");
		a4988_finalize(&dec_driver);
		pigpio_stop(pi);

		return EXIT_FAILURE;
	}

	/*
	  Set up the Push Buttons...
	*/

	for (i = 0; i < (sizeof(pins[0]) / sizeof(int)); ++i) {
		rc = pins_set_mode(pi, pins[0][i], PI_INPUT);
		rc |= pins_set_pull_up_down(pi, pins[0][i], PI_PUD_UP);
		rc |= pins_set_glitch_filter(pi, pins[0][i], 1500);
		callback_id = pins_callback(pi, pins[0][i], FALLING_EDGE,
					    pb_callback);

		if (0 > rc || 0 > callback_id) {
			fprintf(stderr, "Push Button Setup Failed!\n");

			return EXIT_FAILURE;
		}
	}

	for (;;)
		;

	a4988_finalize(&ra_driver);
	a4988_finalize(&dec_driver);
	pigpio_stop(pi);

	return EXIT_SUCCESS;
}
