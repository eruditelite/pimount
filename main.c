/*
  main.c

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

#include "a4988.h"

static struct a4988 driver;
static int pi = -1;

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	/* Try to shut everything down. */

	a4988_finalize(&driver);

	if (-1 != pi)
		pigpio_stop(pi);

	exit(EXIT_FAILURE);
}

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("simple <pins>\n" \
	       "<pins> : A ':' separated list of the gpio pins (8)\n" \
	       "         pin connected to sleep\n" \
	       "         pin connected to direction\n" \
	       "         pin connected to stop\n" \
	       "         pins (3) connected to the micro-step inputs\n" \
	       "         pins (2) used as inputs for control\n");

	exit(exit_code);
}

/*
  ------------------------------------------------------------------------------
  main
*/

int
main(int argc, char *argv[])
{
	int opt = 0;
	int long_index = 0;
	char ip_address[NAME_MAX];
	char ip_port[NAME_MAX];
	unsigned steps = 0;
	unsigned period = 0;
	enum a4988_dir direction = A4988_DIR_CW;
	enum a4988_res resolution = A4988_RES_FULL;
	char *token;
	int pins[5];
	unsigned i;

	static struct option long_options[] = {
		{"steps",      required_argument, 0,  's' },
		{"period",     required_argument, 0,  'p' },
		{"direction",  required_argument, 0,  'd' },
		{"resolution", required_argument, 0,  'r' },
		{"help",       required_argument, 0,  'h' },
		{0,            0,                 0,  0   }
	};

	strcpy(ip_address, "localhost");
	strcpy(ip_port, "8888");

	while ((opt = getopt_long(argc, argv, "s:p:d:r:h", 
				  long_options, &long_index )) != -1) {
		switch (opt) {
		case 's':
			steps = atoi(optarg);
			break;
		case 'p':
			period = atoi(optarg);
			break;
		case 'd':
			if (0 == strncmp("cw", optarg, strlen("cw"))) {
				direction = A4988_DIR_CW;
			} else if (0 == strncmp("ccw", optarg, strlen("ccw"))) {
				direction = A4988_DIR_CCW;
			} else {
				fprintf(stderr, "Invalid Direction\n");
				return EXIT_FAILURE;
			}
			break;
		case 'r':
			if (0 == strncmp("f", optarg, strlen("f"))) {
				resolution = A4988_RES_FULL;
			} else if (0 == strncmp("h", optarg, strlen("h"))) {
				resolution = A4988_RES_HALF;
			} else if (0 == strncmp("q", optarg, strlen("q"))) {
				resolution = A4988_RES_QUARTER;
			} else if (0 == strncmp("e", optarg, strlen("e"))) {
				resolution = A4988_RES_EIGHTH;
			} else {
				fprintf(stderr, "Invalid Resolution\n");
				return EXIT_FAILURE;
			}
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			fprintf(stderr, "Invalid Option\n");
			usage(EXIT_FAILURE);
			break;
		}
	}

	if (1 != (argc - optind)) {
		fprintf(stderr, "GPIO Pins Must Be Specified\n");
		usage(EXIT_FAILURE);
	}

	for (i = 0; i < (sizeof(pins) / sizeof(int)); ++i)
		pins[i] = -1;

	i = 0;
	token = strtok(argv[optind], ":");

	while (NULL != token) {
		pins[i++] = atoi(token);
		token = strtok(NULL, ":");
	}

	for (i = 0; i < (sizeof(pins) / sizeof(int)); ++i) {
		if (-1 == pins[i]) {
			fprintf(stderr, "Invalid GPIO Pin\n");
			return EXIT_FAILURE;
		}

		if (0 == pins[i]) {
			fprintf(stderr, "pigpiod_if2 doesn't allow pin 0!\n");
			return EXIT_FAILURE;
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

	driver.pigpio = pi;
	driver.direction = pins[0];
	driver.step = pins[1];
	driver.sleep = pins[2];
	driver.ms2 = pins[3];
	driver.ms1 = pins[4];

	if (2000 <= period)
		period -= 2000;
	else
		period = 0;

	if (0 != steps) {
		if (0 != a4988_initialize(&driver)) {
			fprintf(stderr, "Initialization Failed\n");

			if (-1 != pi)
				pigpio_stop(pi);

			return EXIT_FAILURE;
		}

		if (0 != a4988_enable(&driver, resolution, direction)) {
			fprintf(stderr, "Stepping Failed\n");

			if (-1 != pi)
				pigpio_stop(pi);

			return EXIT_FAILURE;
		}

		while (0 < steps--) {
			a4988_step(&driver);

			if (0 < period)
				usleep(period);
		}

		a4988_disable(&driver);
		a4988_finalize(&driver);
	}

	pigpio_stop(pi);

	return EXIT_SUCCESS;
}
