/*
  main.c

  == GPIO pins used...

  ra   : 26:19:13:6:5
  dec  : 27:17:4:3:2
  fan  : 18
  port : 0

  The above are based on the current hardware design -- and most were
  chosen to make routing the board easier with one exception.  Since
  hardware PWM is used to control fan speed, 18 is the only safe
  choice.  According to the pigpio documentation, hardware PWM is
  avaialble on pin 18 on all Pi models.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <linux/joystick.h>

#include <pigpio.h>

#include "pimount.h"
#include "server.h"
#include "fan.h"
#include "pins.h"
#include "timespec.h"
#include "stepper.h"
#include "stats.h"

char *cmdErrStr(int);

static pthread_t fan_thread;
static pthread_t controller_thread;
static pthread_t server_thread;

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	stats_finalize();
	stepper_finalize();
	pthread_cancel(server_thread);
	pthread_join(server_thread, NULL);
	pthread_cancel(controller_thread);
	pthread_join(controller_thread, NULL);
	pthread_cancel(fan_thread);
	pthread_join(fan_thread, NULL);

	gpioTerminate();

	pthread_exit(NULL);
}

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(int exit_code)
{
	printf("Usage: pimount\n"
	       "\t--help|-h  Display this wonderful help text...\n");

	exit(exit_code);
}

/*
  ------------------------------------------------------------------------------
  controller
*/

struct controller {
	char joystick[80];
	int joystick_fd;
};

static void
controller_cleanup(void *input)
{
	struct controller *controller_input;

	controller_input = (struct controller *)input;

	if (-1 != controller_input->joystick_fd)
		close(controller_input->joystick_fd);
}

static void
c_track(void)
{
	int rc;

	lock(&state.mutex);

	if (PIMOUNT_CONTROL_LOCAL == state.control &&
	    15.0 == state.ra_rate &&
	    0.0 == state.dec_rate) {
		unlock(&state.mutex);

		return;
	}

	printf("Switching to Local and Tracking!\n");

	state.dec_rate = 0.0;
	state.ra_rate = 15.0;
	state.control = PIMOUNT_CONTROL_LOCAL;

	rc = stepper_stop(STEPPER_AXIS_DEC);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	rc = stepper_stop(STEPPER_AXIS_RA);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	rc = stepper_start(STEPPER_AXIS_RA, state.ra_rate, 0);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	unlock(&state.mutex);

	return;
}

static void
c_stop(void)
{
	int rc;

	lock(&state.mutex);

	if (PIMOUNT_CONTROL_OFF == state.control &&
	    0.0 == state.ra_rate &&
	    0.0 == state.dec_rate) {
		unlock(&state.mutex);

		return;
	}

	printf("Stopping Everything!\n");

	state.ra_rate = 0.0;
	state.dec_rate = 0.0;
	state.control = PIMOUNT_CONTROL_OFF;
	
	rc = stepper_stop(STEPPER_AXIS_DEC);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	rc = stepper_stop(STEPPER_AXIS_RA);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	unlock(&state.mutex);

	return;
}

static void
c_remote(void)
{
	int rc;

	lock(&state.mutex);

	if (PIMOUNT_CONTROL_REMOTE == state.control &&
	    0.0 == state.ra_rate &&
	    0.0 == state.dec_rate) {
		unlock(&state.mutex);

		return;
	}

	printf("Switching to Remote and Stopping!\n");

	state.ra_rate = 0.0;
	state.dec_rate = 0.0;
	state.control = PIMOUNT_CONTROL_REMOTE;

	rc = stepper_stop(STEPPER_AXIS_DEC);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	rc = stepper_stop(STEPPER_AXIS_RA);

	if (rc) {
		unlock(&state.mutex);
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
	}

	unlock(&state.mutex);

	return;
}

static void
c_status(void)
{
	lock(&state.mutex);

	printf("State: %s\n"
	       "ra_rate: %.2f\n"
	       "dec_rate: %.2f\n",
	       pimount_control_names(state.control),
	       state.ra_rate, state.dec_rate);

	unlock(&state.mutex);

	return;
}

static void
c_ra(bool positive)
{
	int rc;
	bool something_changed = false;

	lock(&state.mutex);

	if (PIMOUNT_CONTROL_LOCAL != state.control) {
		fprintf(stderr, "Switch to Local Control First!\n");
		unlock(&state.mutex);

		return;
	}

	if (positive) {
		printf("RA West Pressed: ");

		if (MAX_RA_RATE > state.ra_rate) {
			state.ra_rate += 15.0;
			something_changed = true;
		}

		if (MAX_RA_RATE < state.ra_rate) {
			state.ra_rate = MAX_RA_RATE;
			something_changed = true;
		}

		printf("RA Rate is now %.2f\n", state.ra_rate);
	} else {
		printf("RA East Pressed: ");

		if ((-1.0 * MAX_RA_RATE) < state.ra_rate) {
			state.ra_rate -= 15.0;
			something_changed = true;
		}

		if (MAX_RA_RATE < fabs(state.ra_rate)) {
			state.ra_rate = -1.0 * MAX_RA_RATE;
			something_changed = true;
		}

		printf("RA Rate is now %.2f\n", state.ra_rate);
	}

	if (something_changed) {
		rc = stepper_stop(STEPPER_AXIS_RA);

		if (rc) {
			unlock(&state.mutex);
			fprintf(stderr,
				"%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
		}

		if (0.0 != state.ra_rate) {
			rc = stepper_start(STEPPER_AXIS_RA, state.ra_rate, 0);

			if (rc) {
				unlock(&state.mutex);
				fprintf(stderr,
					"%s:%d - rc=%d\n",
					__FILE__, __LINE__, rc);
			}
		}
	}

	unlock(&state.mutex);

	return;
}

static void
c_dec(bool positive)
{
	int rc;
	bool something_changed = false;

	lock(&state.mutex);

	if (PIMOUNT_CONTROL_LOCAL != state.control) {
		fprintf(stderr, "Switch to Local Control First!\n");
		unlock(&state.mutex);

		return;
	}

	if (positive) {
		printf("DEC North Pressed: ");

		if (MAX_DEC_RATE > state.dec_rate) {
			state.dec_rate += 15.0;
			something_changed = true;
		}

		if (MAX_DEC_RATE < state.dec_rate) {
			state.dec_rate = MAX_DEC_RATE;
			something_changed = true;
		}

		printf("DEC Rate is now %.2f\n", state.dec_rate);
	} else {
		printf("DEC South Pressed: ");

		if ((-1.0 * MAX_DEC_RATE) < state.dec_rate) {
			state.dec_rate -= 15.0;
			something_changed = true;
		}

		if (MAX_DEC_RATE < fabs(state.dec_rate)) {
			state.dec_rate = -1.0 * MAX_DEC_RATE;
			something_changed = true;
		}

		printf("DEC Rate is now %.2f\n", state.dec_rate);
	}

	if (something_changed) {
		rc = stepper_stop(STEPPER_AXIS_DEC);

		if (rc) {
			unlock(&state.mutex);
			fprintf(stderr,
				"%s:%d - rc=%d\n", __FILE__, __LINE__, rc);
		}

		if (0.0 != state.ra_rate) {
			rc = stepper_start(STEPPER_AXIS_DEC, state.dec_rate, 0);

			if (rc) {
				unlock(&state.mutex);
				fprintf(stderr,
					"%s:%d - rc=%d\n",
					__FILE__, __LINE__, rc);
			}
		}
	}

	unlock(&state.mutex);

	return;
}

static void *
controller(void *input)
{
	struct controller *controller_input;

	controller_input = (struct controller *)input;

	controller_input->joystick_fd =
		open(controller_input->joystick, O_RDONLY);

	if (-1 == controller_input->joystick_fd) {
		fprintf(stderr, "open(%s, O_RDONLY) failed: %s\n",
			controller_input->joystick, strerror(errno));

		pthread_exit(NULL);
	}

	pthread_cleanup_push(controller_cleanup, input);

	for (;;) {
		ssize_t bytes;
		struct js_event event;
		static bool active = false;

		bytes = read(controller_input->joystick_fd,
			     &event, sizeof(event));

		if (bytes != sizeof(event)) {
			/* Controller Unplugged... Or Some Such. */
			fprintf(stderr, "Controller Disappeared!\n");
			pthread_exit(NULL);
		}

		if (4 == event.number || 5 == event.number) {
			if (1 == event.value)
				active = true;
			else
				active = false;
		}

		switch (event.type) {
		case JS_EVENT_BUTTON:
			if (0 == event.value) { /* Button pressed. */
				switch (event.number) {
				case 0:
					c_stop();
					break;
				case 1:
					c_track();
					break;
				case 2:
					c_remote();
					break;
				case 3:
					c_status();
					break;
				default:
					/* Ignore other buttons. */
					break;
				}
			}
			break;
		case JS_EVENT_AXIS:
			/* Local search. */
			if (0 != (event.number / 2))
				break;

			if (!active)
				break;

			if (0 == event.number % 2) {
				/* X */
				if (16000 < event.value)
					c_ra(false);
				else if (-16000 > event.value)
					c_ra(true);
				else
					break;
			} else {
				/* Y */
				if (16000 < event.value)
					c_dec(false);
				else if (-16000 > event.value)
					c_dec(true);
				else
					break;
			}
			break;
		default:
			/* Ignore everything else. */
			break;
		}
	}

	if (-1 != controller_input->joystick_fd)
		close(controller_input->joystick_fd);

	pthread_cleanup_pop(1);
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
	struct fan_params fan_input;
	struct server_input server_parameters;
	struct controller controller_input;

	static struct option long_options[] = {
		{"help",      no_argument,       0,  'h' },
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "ha:d:u:r:n:", 
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
	  Catch Signals
	*/

	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGCONT, handler);
	signal(SIGTERM, handler);

	/*
	  Start the Fan Thread
	*/

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
	  Initialize the Stepper Motor Driver
	*/

	rc = stepper_initialize();

	if (rc) {
		gpioTerminate();
		fprintf(stderr, "%s:%d - rc=%d\n", __FILE__, __LINE__, rc);

		return EXIT_FAILURE;
	}

	/*
	  Set up the USB Controller
	*/

	strcpy(controller_input.joystick, JOYSTICK);

	rc = pthread_create(&controller_thread, NULL, controller,
			    (void *)&controller_input);

	if (0 != rc) {
		fprintf(stderr, "pthread_create() failed: %d\n", rc);

		return EXIT_FAILURE;
	}

	/*
	  Start the Server Thread
	*/

	server_parameters.port = 0;

	rc = pthread_create(&server_thread, NULL,
			    server, (void *)&server_parameters);

	if (0 != rc) {
		fprintf(stderr, "pthread_create() failed: %d\n", rc);

		return EXIT_FAILURE;
	}

	rc = pthread_setname_np(server_thread, "pimount.server");

	if (0 != rc)
		fprintf(stderr, "pthread_setname_np() failed: %d\n", rc);

	/*
	  Start 'stats'
	*/

	rc = stats_initialize();

	if (rc) {
		fprintf(stderr, "stats_initialize() failed: %d\n", rc);

		return EXIT_FAILURE;
	}

	/*
	  Pause...
	*/

	pause();

	/*
	  Join all Threads
	*/

	stats_finalize();
	stepper_finalize();
	pthread_join(server_thread, NULL);
	pthread_join(controller_thread, NULL);
	pthread_join(fan_thread, NULL);
	gpioTerminate();

	pthread_exit(NULL);
}
