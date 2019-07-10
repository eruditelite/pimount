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

/*
  ------------------------------------------------------------------------------
  get_temp
*/

static int
get_temp(void)
{
	return 40;
}

/*
  ------------------------------------------------------------------------------
  fan_cleanup
*/

void
fan_cleanup(void *input)
{
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

	sleep.tv_sec = 2;
	sleep.tv_nsec = 0;

	pthread_cleanup_push(fan_cleanup, input);

	for (;;) {
		nanosleep(&sleep, NULL);
		printf("Checking the Temp...\n");
		pthread_testcancel();
	}

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}
