/*
  pimount.c

  Stuff that is widely used, in no particular order.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "pimount.h"

/*
  state is used for local and remote control.
*/

struct pimount_state state = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.control = PIMOUNT_CONTROL_OFF,
	.ra_rate = 0.0,
	.dec_rate = 0.0
};

/*
  ------------------------------------------------------------------------------
  _lock
*/

void
_lock(pthread_mutex_t *mutex, const char *file, int line)
{
	int rc;

	rc = pthread_mutex_lock(mutex);

	if (rc) {
		fprintf(stderr,
			"%s:%d - pthread_mutex_lock() failed: %s\n",
			file, line, strerror(rc));
		exit(EXIT_FAILURE);
	}

	return;
}

/*
  ------------------------------------------------------------------------------
  _unlock
*/

void
_unlock(pthread_mutex_t *mutex, const char *file, int line)
{
	int rc;

	rc = pthread_mutex_unlock(mutex);

	if (rc) {
		fprintf(stderr,
			"%s:%d - pthread_mutex_unlock() failed: %s\n",
			file, line, strerror(rc));
		exit(EXIT_FAILURE);
	}

	return;
}
