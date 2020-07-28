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

#include <pigpio.h>

#include "pimount.h"
#include "stats.h"
#include "oled.h"

/*
  ==============================================================================
  ==============================================================================
  Private
  ==============================================================================
  ==============================================================================
*/

char *cmdErrStr(int); /* For some reason, pigpio doesn't export this! */

static int i2c_handle = -1;
static bool oled_enabled = false;

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

/*
  ------------------------------------------------------------------------------
  update_oled
*/

static void
update_oled(void)
{
	static bool first_run = true;
	int rc;
	int temp;
	long load;
	struct pimount_state state_copy;
	char buffer[80];
	int flen;

	if (!first_run && !oled_enabled)
		return;

	if (first_run) {
		first_run = false;

		i2c_handle = i2cOpen(1, 0x3c, 0);

		if (0 > i2c_handle) {
			fprintf(stderr, "i2cOpen() failed: %s\n",
				cmdErrStr(i2c_handle));

			return;
		} else {
			oled_enabled = true;
		}

		rc = oled_initialize(i2c_handle, false, false);

		if (rc) {
			fprintf(stderr, "oled_initialize() failed: %d\n", rc);
			oled_enabled = false;

			return;
		}

		oled_clear(i2c_handle);
		oled_print(i2c_handle, 0, 0, OLED_FONT_MEDIUM, "PiMount");
		oled_print(i2c_handle, 0, 2, OLED_FONT_MEDIUM, "T/L");
		oled_print(i2c_handle, 0, 4, OLED_FONT_MEDIUM, "R/A");
		oled_print(i2c_handle, 0, 6, OLED_FONT_MEDIUM, "DEC");
	}

	if (!oled_enabled)
		return;

	/* Update State */

	lock(&state.mutex);
	state_copy.control = state.control;
	state_copy.ra_rate = state.ra_rate;
	state_copy.dec_rate = state.dec_rate;
	unlock(&state.mutex);

	/* Update 'control' */

	flen = 15 - 7;	/* Available Space */

	switch (state_copy.control) {
	case PIMOUNT_CONTROL_OFF:
		snprintf(buffer, flen, "Parked");
		break;
	case PIMOUNT_CONTROL_LOCAL:
		snprintf(buffer, flen, "Local");
		break;
	case PIMOUNT_CONTROL_REMOTE:
		snprintf(buffer, flen, "Remote");
		break;
	default:
		break;
	}

	oled_fill(i2c_handle, false, 7, 0, 15, 0);
	oled_print(i2c_handle, 15 - strlen(buffer), 0, OLED_FONT_MEDIUM,
		   buffer);

	flen = 15 - 3;	/* Available space for ra/dec rates. */
	snprintf(buffer, flen, "%.2f", state_copy.ra_rate);
	oled_fill(i2c_handle, false, 3, 4, 15, 4);
	oled_print(i2c_handle, 15 - strlen(buffer), 4, OLED_FONT_MEDIUM,
		   buffer);
	snprintf(buffer, flen, "%.2f", state_copy.dec_rate);
	oled_fill(i2c_handle, false, 3, 6, 15, 6);
	oled_print(i2c_handle, 15 - strlen(buffer), 6, OLED_FONT_MEDIUM,
		   buffer);

	/* Update Temperature and Load */

	load = get_load();
	temp = get_temp();

	flen = 15 - 3;	/* Available space for T/L. */
	snprintf(buffer, flen, "%dC/%ld%%", temp, load);
	oled_fill(i2c_handle, false, 3, 2, 15, 2);
	oled_print(i2c_handle, 15 - strlen(buffer), 2, OLED_FONT_MEDIUM,
		   buffer);
	return;
}

/*
  ------------------------------------------------------------------------------
  pstat
*/

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
				lock(&global.mutex);
				global.load = (this[0] + this[1] + this[2]) -
					(last[0] + last[1] + last[2]);
				global.available = true;
				unlock(&global.mutex);
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

		update_oled();
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

	lock(&global.mutex);

	if (global.available)
		load = global.load;
	else
		load = -1;

	unlock(&global.mutex);

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
	if (oled_enabled) {
		oled_enabled = false;
		oled_clear(i2c_handle);
		oled_finalize(i2c_handle);
	}

	if (0 <= i2c_handle)
		i2cClose(i2c_handle);

	pthread_cancel(pstat_thread);
	pthread_join(pstat_thread, NULL);

	return;
}
