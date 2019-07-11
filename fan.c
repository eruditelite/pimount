/*
  fan.c
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
	struct fan_params *params;

	params = (struct fan_params *)input;

	hardware_PWM(params->pi, params->pin, 100, 0);

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
		nanosleep(&sleep, NULL);
		temp = get_temp();

		if (70 <= temp) {
			/* Above 70 C, run at full speed. */
			rc = hardware_PWM(params->pi,
					  params->pin, 100, 1000000);
		} else if (50 > temp) {
			/* Below 50 C, don't run. */
			rc = hardware_PWM(params->pi,
					  params->pin, 100, 0);
		} else {
			int duty;

			/* From 50 C to 70 C, range from 200000 to 1000000. */
			duty = ((((temp - 50) * 5) *
				 (PI_HW_PWM_RANGE - 200000)) / 100) + 200000;
			rc = hardware_PWM(params->pi,
					  params->pin, 100, duty);
		}

		if (0 > rc)
			fprintf(stderr, "hardware_PWM() failed: %s\n",
				pigpio_error(rc));

		pthread_testcancel();
	}

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}
