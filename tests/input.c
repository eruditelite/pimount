/*
  input.c

  Test the button inputs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>

#include <pigpio.h>

#include "../pins.h"

#define XA  9
#define XB 11
#define YA 12
#define YB 22
#define  Z 10

char *cmdErrStr(int);

static int xa_count = 0;
static int xb_count = 0;
static int ya_count = 0;
static int yb_count = 0;
static int  z_count = 0;

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	printf("Shutting down...\n");
	gpioTerminate();

	exit(EXIT_SUCCESS);
}

/*
  ------------------------------------------------------------------------------
  pin_isr
*/

static void
pin_isr(int gpio, __attribute__((unused)) int level, uint32_t tick)
{
	static uint32_t last_tick;

	if (100000 > (tick - last_tick))
		return;

	switch (gpio) {
	case XA:
		printf("Xa Pressed\n");
		++xa_count;
		break;
	case XB:
		printf("Xb Pressed\n");
		++xb_count;
		break;
	case YA:
		printf("Ya Pressed\n");
		++ya_count;
		break;
 	case YB:
		printf("Yb Pressed\n");
		++yb_count;
		break;
 	case Z:
		printf("Z Pressed\n");
		++z_count;
		break;
	default:
		break;
	}

	last_tick = tick;

	return;
}

/*
  ------------------------------------------------------------------------------
  setup_pin
*/

static int
setup_pin(int gpio)
{
	int rc;

	rc = gpioSetMode(gpio, PI_INPUT);

	if (0 != rc) {
		fprintf(stderr, "gpioSetMode() failed: %s\n",
			cmdErrStr(rc));
		gpioTerminate();

		return EXIT_FAILURE;
	}

	rc = gpioSetPullUpDown(gpio, PI_PUD_UP);

	if (0 != rc) {
		fprintf(stderr, "gpioSetPullUpDown() failed: %s\n",
			cmdErrStr(rc));
		gpioTerminate();

		return EXIT_FAILURE;
	}

	rc = gpioSetISRFunc(gpio, FALLING_EDGE, 0, pin_isr);

	if (0 != rc) {
		fprintf(stderr, "gpioSetISRFunc() failed: %s\n",
			cmdErrStr(rc));
		gpioTerminate();

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
  ------------------------------------------------------------------------------
  main
*/

int
main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
	int rc;

	/*
	  Initialize
	*/

	rc = gpioInitialise();

	if (PI_INIT_FAILED == rc) {
		fprintf(stderr, "gpioinitialise() failed: %s\n", cmdErrStr(rc));

		return EXIT_FAILURE;
	}

	/*
	  Catch Signals -- handler Will Finalize pigpio
	*/

	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGCONT, handler);
	signal(SIGTERM, handler);

	/*
	  Set Up Pins
	*/

	if (EXIT_SUCCESS != setup_pin(Z) ||
	    EXIT_SUCCESS != setup_pin(XA) ||
	    EXIT_SUCCESS != setup_pin(XB) ||
	    EXIT_SUCCESS != setup_pin(YA) ||
	    EXIT_SUCCESS != setup_pin(YB)) {
		fprintf(stderr, "setup_pin() failed...\n");

		return EXIT_FAILURE;
	}

	/*
	  Sleep
	*/

	for (;;)
		pause();

	return EXIT_SUCCESS;
}
