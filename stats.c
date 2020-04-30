/*
  ==============================================================================
  ==============================================================================
  stats.c
  ==============================================================================
  ==============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "stats.h"

/*
  ==============================================================================
  ==============================================================================
  Private
  ==============================================================================
  ==============================================================================
*/

static pthread_t pstat_thread;

struct stats {
	pthread_mutex_t mutex;
	bool available;
	long load;
};

static struct stats global = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.available = false,
};

/* If a Lock Fails, Die Horribly */

static inline void
_lock(const char *file, int line)
{
	int rc;

	rc = pthread_mutex_lock(&global.mutex);

	if (rc) {
		fprintf(stderr,
			"%s:%d - pthread_mutex_lock() failed: %s\n",
			file, line, strerror(rc));
		exit(EXIT_FAILURE);
	}

	return;
}

#define lock() _lock(__FILE__, __LINE__);

static inline void
_unlock(const char *file, int line)
{
	int rc;

	rc = pthread_mutex_unlock(&global.mutex);

	if (rc) {
		fprintf(stderr,
			"%s:%d - pthread_mutex_unlock() failed: %s\n",
			file, line, strerror(rc));
		exit(EXIT_FAILURE);
	}

	return;
}

#define unlock() _unlock(__FILE__, __LINE__);

static void *
pstat(__attribute__((unused)) void *input)
{
	int rc;
	long last[3];
	long this[3];
	struct timespec last_stamp;
	const struct timespec sleep = {
		.tv_sec = 1,
		.tv_nsec = 0
	};
	bool first_run = true;
	char id[16];

	rc = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setcanceltype() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		pthread_exit(NULL);
	}

	rc = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	if (rc) {
		fprintf(stderr, "%s:%d - pthread_setcancelstate() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		pthread_exit(NULL);
	}

	/*
	  This is the main loop -- now that everything has been set up.
	*/

	for (;;) {
		FILE *pstat;

		/* Check for Cancellation */
		pthread_testcancel();

		pstat = fopen("/proc/stat", "r");

		if (NULL == pstat) {
			fprintf(stderr, "%s:%d - fopen() failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
			nanosleep(&sleep, NULL);
			continue;
		}

		rc = clock_gettime(CLOCK_MONOTONIC_COARSE, &last_stamp);

		if (-1 == rc) {
			fprintf(stderr, "%s:%d - clock_gettime() failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
			nanosleep(&sleep, NULL);
			continue;
		}

		rc = fscanf(pstat, "%s %ld %ld %ld",
			    id, &this[0], &this[1], &this[2]);

		if (4 != rc) {
			if (ferror(pstat)) {
				fprintf(stderr, "%s:%d - fscanf() failed: %s\n",
					__FILE__, __LINE__, strerror(errno));
			} else if (feof(pstat)) {
				fprintf(stderr, "%s:%d - fscanf() hit EOF\n",
					__FILE__, __LINE__);
			} else {
				fprintf(stderr, "%s:%d - fscanf() failed\n",
					__FILE__, __LINE__);
			}
		} else {
			if(first_run) {
				first_run = false;
			} else {
				lock();
				global.load = (this[0] + this[1] + this[2]) -
					(last[0] + last[1] + last[2]);
				global.available = true;
				unlock();
			}

			last[0] = this[0];
			last[1] = this[1];
			last[2] = this[2];
		}

		fclose(pstat);

		rc = nanosleep(&sleep, NULL);

		if (rc) {
			fprintf(stderr,	"%s:%d - nanosleep(%ld %ld) failed: %s\n",
				__FILE__, __LINE__,
				sleep.tv_sec, sleep.tv_nsec, strerror(errno));
		}
	}

	pthread_exit(NULL);
}

/*
  ==============================================================================
  ==============================================================================
  Public
  ==============================================================================
  ==============================================================================
*/

/*
  ------------------------------------------------------------------------------
  get_temp

  Returns the temperature in degrees centigrade.
*/

int
get_temp(void)
{
	FILE *thermal;
	int temp;
	int rc;

	thermal = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

	if (NULL == thermal) {
		fprintf(stderr, "%s:%d - fopen() failed: %s\n",
			__FILE__, __LINE__, strerror(errno));

		return -1;
	}

	rc = fscanf(thermal, "%d", &temp);

	if (EOF == rc) {
		if (ferror(thermal))
			fprintf(stderr, "%s:%d - fscanf() failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
		else
			fprintf(stderr, "%s:%d - fscanf() returned EOF\n",
				__FILE__, __LINE__);

		return -1;
	}

	fclose(thermal);

	return temp / 1000;
}

/*
  ------------------------------------------------------------------------------
  get_load
*/

long
get_load(void)
{
	long load;

	lock();

	if (global.available)
		load = global.load;
	else
		load = -1;

	unlock();

	return load;
}

/*
  ------------------------------------------------------------------------------
  stats_initialize
*/

int
stats_initialize(void)
{
	int rc;

	rc = pthread_create(&pstat_thread, NULL, pstat, NULL);

 	if (rc) {
		fprintf(stderr, "%s:%d - pthread_create() failed: %s\n",
			__FILE__, __LINE__, strerror(rc));

		return -1;
	}

	rc = pthread_setname_np(pstat_thread, "pimount.pstat");

 	if (rc)
		fprintf(stderr, "%s:%d - pthread_setname_np() failed\n",
			__FILE__, __LINE__);

	return 0;
}

/*
  ------------------------------------------------------------------------------
  stats_finalize
*/

void
stats_finalize(void)
{
	return;
}
