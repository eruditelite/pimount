/*
  ==============================================================================
  ==============================================================================

  input.c

  See https://www.kernel.org/doc/Documentation/input/joystick-api.txt.

  Use Axis 0 to handle local searches.
  Use Button 0 to stop everything (what INDI calls Park?).
  Use Button 1 to start tracking.

  ==============================================================================
  ==============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <linux/joystick.h>

static int js_fd = -1;
static char *js_name = "/dev/input/js0";

/*
  ------------------------------------------------------------------------------
  clean_up
*/

static void
clean_up(void)
{
	printf("Shutting Down Input USB Test...\n");

	if (-1 != js_fd)
		close(js_fd);
}

/*
  ------------------------------------------------------------------------------
  handler
*/

static void
handler(__attribute__((unused)) int signal)
{
	clean_up();
	exit(EXIT_SUCCESS);
}

/*
  ------------------------------------------------------------------------------
  print_identity
*/

static int
print_identity(int fd)
{
	/* As linux/joystick.h doesn't limit the size... */
	char id_string[128];
	int version = -1;
	char buttons = -1;
	char axes = -1;

	if (-1 == ioctl(fd, JSIOCGNAME(sizeof(id_string)), id_string)) {
		fprintf(stderr, "ioctl() failed: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	if (-1 == ioctl(fd, JSIOCGVERSION, &version)) {
		fprintf(stderr, "ioctl() failed: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	if (-1 == ioctl(fd, JSIOCGBUTTONS, &buttons)) {
		fprintf(stderr, "ioctl() failed: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	if (-1 == ioctl(fd, JSIOCGAXES, &axes)) {
		fprintf(stderr, "ioctl() failed: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	printf("%s\nVersion: 0x%x\n%d Buttons, %d Axes\n",
	       id_string, version, buttons, axes);

	return EXIT_SUCCESS;
}

static void
handle_event(struct js_event *event)
{
	int index = -1;
	const char *event_names[] = {
		"JS_EVENT_BUTTON",
		"JS_EVENT_AXIS",
		"JS_EVENT_INIT|JS_EVENT_INIT",
		"JS_EVENT_INIT|JS_EVENT_AXIS"
	};

	switch (event->type) {
	case JS_EVENT_BUTTON:
		index = 0;
		break;
	case JS_EVENT_AXIS:
		index = 1;
		break;
	case (JS_EVENT_INIT | JS_EVENT_BUTTON):
		index = 2;
		break;
	case (JS_EVENT_INIT | JS_EVENT_AXIS):
		index = 3;
		break;
	default:
		fprintf(stderr, "Unknown Event Type: 0x%x/%u\n",
			event->type, event->type);
		break;
	}

	if (-1 != index) {
		printf("%s (%u ms) number=%u value=%d\n",
		       event_names[index],
		       event->time, event->number, event->value);

		/*
		  Only handle button press events, and treat axis 0
		  and 1 as buttons.

		  This is based on using the iBuffalo Classic USB Gamepad.
		*/

		if (JS_EVENT_BUTTON == event->type && 1 == event->value) {
			if (0 == event->number)
				/* Button 0 Means STOP */
				printf("Stop Everything\n");
			else if (1 == event->number)
				/* Button 1 Means Local Control and Track */
				printf("Local Control and Track\n");
			else if (2 == event->number)
				/* Button 2 Means Remote Control and Stop */
				printf("Remote Control and Stop\n");
			else if (3 == event->number)
				/* Button 3 Means Dump Status to Console */
				printf("Dump Status\n");
		} else if (JS_EVENT_AXIS == event->type) {
			int which;
			int axis;

			/*
			  Which control on the controller (there may
			  be more than one) can be determined by
			  dividing the number by 2 since each control
			  will have X and Y).
			*/

			which = (event->number / 2);

			/*
			  Which axis, x or y, can be found with number % 2.
			*/

			axis = (event->number % 2);

			if (0 == which) {
				if (0 == axis && event->value == 32767)
					printf("RA East\n");
				if (0 == axis && event->value == -32767)
					printf("RA West\n");
				if (1 == axis && event->value == 32767)
					printf("DEC South\n");
				if (1 == axis && event->value == -32767)
					printf("DEC North\n");
			}
		}
	}

	return;
}

/*
  ------------------------------------------------------------------------------
  main
*/

int
main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
	js_fd = open(js_name, O_RDONLY);

	if (-1 == js_fd) {
		fprintf(stderr, "open(%s, O_RDONLY) failed: %s\n",
			js_name, strerror(errno));

		return EXIT_FAILURE;
	}

	printf("Using %s\n", js_name);
	print_identity(js_fd);

	/*
	  Catch Signals
	*/

	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGCONT, handler);
	signal(SIGTERM, handler);

	for (;;) {
		ssize_t bytes;
 		struct js_event event;

		bytes = read(js_fd, &event, sizeof(event));

		if (bytes != sizeof(event))
			break;

		handle_event(&event);
	}

	clean_up();

	return EXIT_SUCCESS;
}
