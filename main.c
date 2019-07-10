/*
  main.c

  == Defaults...

  buttons : 0:22:11:9:10
  ra : 26:19:13:6:5
  dec : 27:17:4:3:2
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <pigpiod_if2.h>

#include "pins.h"
#include "a4988.h"

static int pi = -1;

static pthread_t ra_thread;
static pthread_t dec_thread;
static pthread_t fan_thread;

struct speed {
	int state;
	enum a4988_res resolution;
	enum a4988_dir direction;
	unsigned delay;		/* micro seconds */
};

static struct speed ra_speeds[] = {
	{1, A4988_RES_FULL, A4988_DIR_CCW, 0},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 0},
	{1, A4988_RES_QUARTER, A4988_DIR_CCW, 0},
	{1, A4988_RES_EIGHTH, A4988_DIR_CCW, 0},
	{1, A4988_RES_EIGHTH, A4988_DIR_CCW, 30000}, /* Earth Rotation Speed */
	{1, A4988_RES_EIGHTH, A4988_DIR_CW, 0},
	{1, A4988_RES_QUARTER, A4988_DIR_CW, 0},
	{1, A4988_RES_HALF, A4988_DIR_CW, 0},
	{1, A4988_RES_FULL, A4988_DIR_CW, 0},
};

#define RA_SPEED_DEFAULT 4
#define RA_SPEED_MAX 8

static struct speed dec_speeds[] = {
	{1, A4988_RES_FULL, A4988_DIR_CCW, 0},
	{1, A4988_RES_HALF, A4988_DIR_CCW, 0},
	{1, A4988_RES_QUARTER, A4988_DIR_CCW, 0},
	{1, A4988_RES_EIGHTH, A4988_DIR_CCW, 0},
	{0, A4988_RES_EIGHTH, A4988_DIR_CCW, 0}, /* OFF */
	{1, A4988_RES_EIGHTH, A4988_DIR_CW, 0},
	{1, A4988_RES_QUARTER, A4988_DIR_CW, 0},
	{1, A4988_RES_HALF, A4988_DIR_CW, 0},
	{1, A4988_RES_FULL, A4988_DIR_CW, 0},
};

#define DEC_SPEED_DEFAULT 4
#define DEC_SPEED_MAX 8

struct motion {
	pthread_mutex_t mutex;
	struct a4988 driver;
	struct speed *speed;
	int current_speed;
	int max_speed;
};

static struct motion ra;
static struct motion dec;

static unsigned pb_pins[5];

#define PB_UP (pb_pins[0])
#define PB_DOWN (pb_pins[1])
#define PB_LEFT (pb_pins[2])
#define PB_RIGHT (pb_pins[3])
#define PB_OFF (pb_pins[4])

/*
  ------------------------------------------------------------------------------
  stepper_cleanup
*/

static void
stepper_cleanup(void *input)
{
	struct motion *motion;

	motion = (struct motion *)input;
	printf("%s:%d - Finalizing %s\n",
	       __FILE__, __LINE__, motion->driver.description);
	a4988_finalize(&(motion->driver));
}

/*
  ------------------------------------------------------------------------------
  stepper
*/

static void *
stepper(void *input)
{
	struct motion *motion;
	struct a4988 *driver;
	struct speed *speed;
	int current_speed;

	motion = (struct motion *)input;
	pthread_mutex_lock(&motion->mutex);
	driver = &motion->driver;
	speed = motion->speed;
	current_speed = motion->current_speed;
	speed += current_speed;
	pthread_cleanup_push(stepper_cleanup, input);

	/* First run... */
	if (1 == speed->state)
		a4988_enable(driver, speed->resolution, speed->direction);

	for (;;) {
		struct timespec start;
		struct timespec now;
		struct timespec sleep;

		clock_gettime(CLOCK_MONOTONIC, &start);

		/*
		  See if Anything Changed, Update if Needed
		*/
		  
		if (current_speed != motion->current_speed) {
			printf("%s:%d - %s Update %d %d\n",
			       __FILE__, __LINE__, driver->description,
			       current_speed, motion->current_speed);
			speed = motion->speed;
			current_speed = motion->current_speed;
			speed += current_speed;
			a4988_disable(driver);

			if (1 == speed->state)
				a4988_enable(driver, speed->resolution,
					     speed->direction);
		}

		pthread_testcancel();

		if (1 == speed->state) {
			a4988_step(driver);

			/* calculate sleep time... */
			clock_gettime(CLOCK_MONOTONIC, &now);

			if (now.tv_sec > start.tv_sec)
				now.tv_nsec += 1000000000;

			sleep.tv_sec = 0;
			sleep.tv_nsec = (speed->delay * 1000)
				- (now.tv_nsec - start.tv_nsec)
				- 2000000;
		} else {
			sleep.tv_sec = 1;
			sleep.tv_nsec = 0;
		}

		pthread_mutex_unlock(&motion->mutex);
		nanosleep(&sleep, NULL);
		pthread_mutex_lock(&motion->mutex);
	}

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	pthread_cancel(fan_thread);
	pthread_join(fan_thread, NULL);
	pthread_cancel(ra_thread);
	pthread_join(ra_thread, NULL);
	pthread_cancel(dec_thread);
	pthread_join(dec_thread, NULL);

	if (-1 != pi)
		pigpio_stop(pi);

	pthread_exit(NULL);
}

/*
  ------------------------------------------------------------------------------
  pb_callback
*/

static void
pb_callback(__attribute__((unused)) int pi,
	    unsigned int pin,
	    __attribute__((unused)) unsigned int level,
	    __attribute__((unused)) unsigned int tick)
{
	if (PB_OFF == pin) {
		/*
		  Set to follow the Earth's rotation.
		*/

		/* DEC off */
		pthread_mutex_lock(&dec.mutex);
		dec.current_speed = DEC_SPEED_DEFAULT;
		pthread_mutex_unlock(&dec.mutex);

		/* RA at Earth speed */
		pthread_mutex_lock(&ra.mutex);
		ra.current_speed = RA_SPEED_DEFAULT;
		pthread_mutex_unlock(&ra.mutex);
	} else if (PB_UP == pin || PB_DOWN == pin) {
		/*
		  Increase or Decrease the DEC Motion
		*/

		pthread_mutex_lock(&dec.mutex);

		if (PB_UP == pin && dec.current_speed < dec.max_speed)
			++ dec.current_speed;
		else if (PB_DOWN == pin && dec.current_speed > 0)
			-- dec.current_speed;

		pthread_mutex_unlock(&dec.mutex);
	} else if (pin == pb_pins[2] || pin == pb_pins[3]) {
		/*
		  Increase or Decrease the DEC Motion
		*/

		pthread_mutex_lock(&ra.mutex);

		if (PB_LEFT == pin && ra.current_speed > 0)
			-- ra.current_speed;
		else if (PB_RIGHT == pin && ra.current_speed < ra.max_speed)
			++ ra.current_speed;

		pthread_mutex_unlock(&ra.mutex);
	}

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
	  Start the Fan Thread
	*/

	rc = pthread_create(&fan_thread, NULL, fan, NULL);

	if (0 != rc) {
		fprintf(stderr, "pthread_create() failed: %d\n", rc);

		return EXIT_FAILURE;
	}
	/*
	  Initialize the Stepper Motor Drivers
	*/

	strcpy(ra.driver.description, "RA Driver");
	ra.driver.pigpio = pi;
	ra.driver.direction = pins[1][0];
	ra.driver.step = pins[1][1];
	ra.driver.sleep = pins[1][2];
	ra.driver.ms2 = pins[1][3];
	ra.driver.ms1 = pins[1][4];
	ra.speed = &ra_speeds[0];
	ra.current_speed = RA_SPEED_DEFAULT;
	ra.max_speed = RA_SPEED_MAX;
	pthread_mutex_init(&ra.mutex, NULL);
	pthread_mutex_lock(&ra.mutex);

	printf("RA Pins: d/st/sl/2/1 = %d/%d/%d/%d/%d\n",
	       ra.driver.direction, ra.driver.step, ra.driver.sleep,
	       ra.driver.ms2, ra.driver.ms1);

	if (0 != a4988_initialize(&(ra.driver))) {
		fprintf(stderr, "RA Initialization Failed!\n");
		pigpio_stop(pi);

		return EXIT_FAILURE;
	} else {
		rc = pthread_create(&ra_thread, NULL, stepper,
				    (void *)&ra);

		if (0 != rc) {
			fprintf(stderr, "pthread_create() failed: %d\n", rc);

			return EXIT_FAILURE;
		}
	}

	pthread_mutex_unlock(&ra.mutex);

	strcpy(dec.driver.description, "DEC Driver");
	dec.driver.pigpio = pi;
	dec.driver.direction = pins[2][0];
	dec.driver.step = pins[2][1];
	dec.driver.sleep = pins[2][2];
	dec.driver.ms2 = pins[2][3];
	dec.driver.ms1 = pins[2][4];
	dec.speed = &dec_speeds[0];
	dec.current_speed = DEC_SPEED_DEFAULT;
	dec.max_speed = DEC_SPEED_MAX;
	pthread_mutex_init(&dec.mutex, NULL);
	pthread_mutex_lock(&dec.mutex);

	if (0 != a4988_initialize(&(dec.driver))) {
		fprintf(stderr, "DEC Initialization Failed!\n");
		a4988_finalize(&(ra.driver));
		pigpio_stop(pi);

		return EXIT_FAILURE;
	} else {
		rc = pthread_create(&dec_thread, NULL, stepper,
				    (void *)&dec);

		if (0 != rc) {
			fprintf(stderr, "pthread_create() failed: %d\n", rc);

			return EXIT_FAILURE;
		}
	}

	pthread_mutex_unlock(&dec.mutex);

	/*
	  Set up the Push Buttons...
	*/

	for (i = 0; i < (sizeof(pins[0]) / sizeof(int)); ++i) {
		rc = pins_set_mode(pi, pins[0][i], PI_INPUT);
		rc |= pins_set_pull_up_down(pi, pins[0][i], PI_PUD_UP);
		rc |= pins_set_glitch_filter(pi, pins[0][i], 50);
		callback_id = pins_callback(pi, pins[0][i], FALLING_EDGE,
					    pb_callback);

		if (0 > rc || 0 > callback_id) {
			fprintf(stderr, "Push Button Setup Failed!\n");

			return EXIT_FAILURE;
		}

		pb_pins[i] = pins[0][i];
	}

	pthread_join(fan_thread, NULL);
	pthread_join(ra_thread, NULL);
	pthread_join(dec_thread, NULL);
	a4988_finalize(&(ra.driver));
	a4988_finalize(&(dec.driver));
	pigpio_stop(pi);

	pthread_exit(NULL);
}
