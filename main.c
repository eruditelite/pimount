/*
  main.c

  == GPIO pins used...

  buttons : 0:22:11:9:10
  ra : 26:19:13:6:5
  dec : 27:17:4:3:2
  fan : 18

  The above are based on the current hardware design -- and most were
  chosen to make routing the board easier with one exception.  Since
  hardware PWM is used to control fan speed, 18 is the only safe
  choice.  According to the pigpio documentation, hardware PWM is
  avaialble on pin 18 on all Pi models.
*/

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <pigpio.h>

#include "control.h"
#include "fan.h"
#include "pins.h"
#include "a4988.h"
#include "timespec.h"
#include "stepper.h"

char *cmdErrStr(int);

static struct speed ra_speed;
static struct motion ra_motion;
static struct speed dec_speed;
static struct motion dec_motion;

static unsigned pb_pins[5];

#define PB_UP (pb_pins[0])
#define PB_DOWN (pb_pins[1])
#define PB_LEFT (pb_pins[2])
#define PB_RIGHT (pb_pins[3])
#define PB_OFF (pb_pins[4])

static pthread_t ra_thread;
static pthread_t dec_thread;
static pthread_t fan_thread;
static pthread_t control_thread;

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	pthread_cancel(control_thread);
	pthread_join(control_thread, NULL);
	pthread_cancel(fan_thread);
	pthread_join(fan_thread, NULL);
	pthread_cancel(ra_thread);
	pthread_join(ra_thread, NULL);
	pthread_cancel(dec_thread);
	pthread_join(dec_thread, NULL);
	gpioTerminate();

	pthread_exit(NULL);
}

/*
  ------------------------------------------------------------------------------
  pb_isr
*/

static void
pb_isr(int gpio, __attribute__((unused)) int level, uint32_t tick)
{
	int rc;
	static uint32_t last_tick;
	int ra_change = 0;
	int dec_change = 0;

	if (100000 > (tick - last_tick))
		return;

	if (PB_OFF == (unsigned)gpio) {
		/*
		  Set to follow the Earth's rotation.
		*/

		/* DEC off */
		dec_motion.speed->state = MOTION_STATE_OFF;

		/* RA at Earth speed */
		rc = stepper_set_rate(ra_motion.speed,
				      MOTION_STATE_ON,
				      MOTION_AXIS_RA,
				      MOTION_DIR_NW,
				      15.0);

		if (EXIT_SUCCESS != rc)
			fprintf(stderr, "stepper_set_rate() failed!\n");

		ra_change = 1;
		dec_change = 1;
	} else if (PB_UP == (unsigned)gpio || PB_DOWN == (unsigned)gpio) {
		/*
		  Increase or Decrease the DEC Motion
		*/

		dec_change = 1;
	} else if ((unsigned)gpio == pb_pins[2] ||
		   (unsigned)gpio == pb_pins[3]) {
		/*
		  Increase or Decrease the DEC Motion
		*/

		ra_change = 1;
	}

	if (0 != ra_change)
		pthread_cond_signal(&ra_motion.cond);

	if (0 != dec_change)
		pthread_cond_signal(&dec_motion.cond);

	return;
}

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("pimount <button pins> <ra pins> <da pins> <fan pin> <control port>\n" \
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
	       "         pin connected to ms1\n" \
	       "<fan pin> : A pin to control the cooling fan\n" \
	       "<control port>: The control server port\n");

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
	char *token;
	unsigned i;
	unsigned j;
	int pins[3][5];
	int fan_pin;
	int control_port;
	struct fan_params fan_input;
	struct control_input control_parameters;

	static struct option long_options[] = {
		{"help",       required_argument, 0,  'h' },
		{0,            0,                 0,  0   }
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

	if (5 != (argc - optind)) {
		fprintf(stderr,
			"GPIO Pins and Control Port Must Be Specified\n");
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

	fan_pin = atoi(argv[optind++]);

	control_port = atoi(argv[optind++]);

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
	  Start the Fan Thread
	*/

	fan_input.pin = fan_pin;
	fan_input.bias = 200000;
	fan_input.high = 70;
	fan_input.low = 50;

	rc = pthread_create(&fan_thread, NULL, fan, (void *)&fan_input);

	if (0 != rc) {
		fprintf(stderr, "pthread_create() failed: %d\n", rc);

		return EXIT_FAILURE;
	}

	rc = pthread_setname_np(fan_thread, "pimount.fan");

	if (0 != rc)
		fprintf(stderr, "pthread_setname_np() failed: %d\n", rc);

	/*
	  Initialize the Stepper Motor Drivers
	*/

	strcpy(ra_motion.driver.description, "RA Driver");
	ra_motion.driver.direction = pins[1][0];
	ra_motion.driver.step = pins[1][1];
	ra_motion.driver.sleep = pins[1][2];
	ra_motion.driver.ms2 = pins[1][3];
	ra_motion.driver.ms1 = pins[1][4];
	ra_motion.speed = &ra_speed;

	if (EXIT_SUCCESS != stepper_set_rate(ra_motion.speed,
					     MOTION_STATE_ON,
					     MOTION_AXIS_RA,
					     MOTION_DIR_NW,
					     15.0)) {
		fprintf(stderr, "stepper_set_speed() failed!\n");

		return EXIT_FAILURE;
	}

	pthread_mutex_init(&ra_motion.mutex, NULL);
	pthread_mutex_lock(&ra_motion.mutex);
	pthread_cond_init(&ra_motion.cond, NULL);

	printf("RA Pins: d/st/sl/2/1 = %d/%d/%d/%d/%d\n",
	       ra_motion.driver.direction, ra_motion.driver.step,
	       ra_motion.driver.sleep, ra_motion.driver.ms2,
	       ra_motion.driver.ms1);

	if (0 != a4988_initialize(&(ra_motion.driver))) {
		fprintf(stderr, "RA Initialization Failed!\n");
		gpioTerminate();

		return EXIT_FAILURE;
	} else {
		rc = pthread_create(&ra_thread, NULL, stepper,
				    (void *)&ra_motion);

		if (0 != rc) {
			fprintf(stderr, "pthread_create() failed: %d\n", rc);

			return EXIT_FAILURE;
		}

		rc = pthread_setname_np(ra_thread, "pimount.ra");

		if (0 != rc)
			fprintf(stderr,
				"pthread_setname_np() failed: %d\n", rc);
	}

	pthread_mutex_unlock(&ra_motion.mutex);

	strcpy(dec_motion.driver.description, "DEC Driver");
	dec_motion.driver.direction = pins[2][0];
	dec_motion.driver.step = pins[2][1];
	dec_motion.driver.sleep = pins[2][2];
	dec_motion.driver.ms2 = pins[2][3];
	dec_motion.driver.ms1 = pins[2][4];
	dec_motion.speed = &dec_speed;

	if (EXIT_SUCCESS != stepper_set_rate(dec_motion.speed,
					     MOTION_STATE_OFF,
					     MOTION_AXIS_DEC,
					     MOTION_DIR_NW,
					     0.0)) {
		fprintf(stderr, "stepper_set_speed() failed!\n");

		return EXIT_FAILURE;
	}

	pthread_mutex_init(&dec_motion.mutex, NULL);
	pthread_mutex_lock(&dec_motion.mutex);
	pthread_cond_init(&dec_motion.cond, NULL);

	if (0 != a4988_initialize(&(dec_motion.driver))) {
		fprintf(stderr, "DEC Initialization Failed!\n");
		a4988_finalize(&(ra_motion.driver));
		gpioTerminate();

		return EXIT_FAILURE;
	} else {
		rc = pthread_create(&dec_thread, NULL, stepper,
				    (void *)&dec_motion);

		if (0 != rc) {
			fprintf(stderr, "pthread_create() failed: %d\n", rc);

			return EXIT_FAILURE;
		}

		rc = pthread_setname_np(dec_thread, "pimount.dec");

		if (0 != rc)
			fprintf(stderr,
				"pthread_setname_np() failed: %d\n", rc);
	}

	pthread_mutex_unlock(&dec_motion.mutex);

	/*
	  Set up the Push Buttons...
	*/

	for (i = 0; i < (sizeof(pins[0]) / sizeof(int)); ++i) {
		rc = pins_set_mode(pins[0][i], PI_INPUT);
		rc |= pins_set_pull_up_down(pins[0][i], PI_PUD_UP);
		rc |= pins_isr(pins[0][i], FALLING_EDGE, 0, pb_isr);

		if (0 > rc) {
			fprintf(stderr, "Push Button Setup Failed!\n");

			return EXIT_FAILURE;
		}

		pb_pins[i] = pins[0][i];
	}

	/*
	  Start the Control Thread
	*/

	control_parameters.port = control_port;

	rc = pthread_create(&control_thread, NULL,
			    control, (void *)&control_parameters);

	if (0 != rc) {
		fprintf(stderr, "pthread_create() failed: %d\n", rc);

		return EXIT_FAILURE;
	}

	rc = pthread_setname_np(control_thread, "pimount.control");

	if (0 != rc)
		fprintf(stderr, "pthread_setname_np() failed: %d\n", rc);

	pthread_join(control_thread, NULL);
	pthread_join(fan_thread, NULL);
	pthread_join(ra_thread, NULL);
	pthread_join(dec_thread, NULL);
	a4988_finalize(&(ra_motion.driver));
	a4988_finalize(&(dec_motion.driver));
	gpioTerminate();

	pthread_exit(NULL);
}
