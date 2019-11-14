/*
  fan.c

  The fan thread.
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

#include "fan.h"

char *cmdErrStr(int);

/*
  ------------------------------------------------------------------------------
  get_temp
*/

static int
get_temp(void)
{
	FILE *thermal;
	int temp;

	thermal = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	(void)fscanf(thermal, "%d", &temp);
	fclose(thermal);
	temp /= 1000;

	return temp;
}

/*
  ------------------------------------------------------------------------------
  fan_cleanup
*/

void
fan_cleanup(void *input)
{	
	int rc;
	struct fan_params *params;

	params = (struct fan_params *)input;

	rc = gpioHardwarePWM(params->pin, 100, 0);

	if (0 > rc)
		fprintf(stderr, "gpioHardwarePWM() failed: %s\n",
			cmdErrStr(rc));

	return;
}

/*
  ------------------------------------------------------------------------------
  fan
*/

void *
fan(void *input)
{
	struct timespec sleep;
	struct fan_params *params;
	int temp;
	int rc;

	params = (struct fan_params *)input;

	sleep.tv_sec = 5;
	sleep.tv_nsec = 0;

	pthread_cleanup_push(fan_cleanup, input);

	for (;;) {
		int duty;

		nanosleep(&sleep, NULL);
		temp = get_temp();

		if (params->high < temp) {
			/* Above 'high', run at full speed. */
			duty = PI_HW_PWM_RANGE;
		} else if (params->low > temp) {
			/* Below 'low', don't run. */
			duty = 0;
		} else {
			/*
			  Between 'low' and 'high', range from 'bias'
			  to 1,000,000.
			*/

			duty = ((((temp - params->low) * 5) *
				 (PI_HW_PWM_RANGE - params->bias)) / 100) +
				params->bias;
		}

		rc = gpioHardwarePWM(params->pin, 100, duty);

		if (0 > rc)
			fprintf(stderr, "hardware_PWM() failed: %s\n",
				cmdErrStr(rc));

		pthread_testcancel();
	}

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}
