/*
  pin.c
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include <pigpiod_if2.h>

char *cmdErrStr(int);

static int default_pin = 18;
static int default_frequency = 100;
static int default_duty = 50;

/*
  ------------------------------------------------------------------------------
  usage
*/

static void
usage(const char *name, int exit_code)
{
	printf("Usage: %s [-h] -p <number> -f <number> -d <number>\n", name);
	printf("\t-h --help         Display help\n"
	       "\t-p --pin          GPIO (BCM) Pin Number [%d default]\n"
	       "\t-f --frequency    Frequency in Hz (0...30 KHz) [%d default]\n"
	       "\t-d --duty         Duty Cycle (0...100 %%) [%d default]\n",
	       default_pin, default_frequency, default_duty);

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
	char ip_address[NAME_MAX];
	char ip_port[NAME_MAX];
	int pin = default_pin;
	int frequency = default_frequency;;
	int duty = default_duty;
	int c;

	static const struct option lopts[] = {
		{ "help",      0, 0, 'h' },
		{ "pin",       1, 0, 'p' },
		{ "frequency", 1, 0, 'f' },
		{ "duty",      1, 0, 'd' },
		{ NULL, 0, 0, 0 },
	};

	while (-1 != (c = getopt_long(argc, argv, "hp:f:d:", lopts, NULL))) {

		switch (c) {
		case 'p':
			pin = strtol(optarg, NULL, 0);
			break;
		case 'f':
			frequency = strtol(optarg, NULL, 0);
			break;
		case 'd':
			duty = strtol(optarg, NULL, 0);
			break;
		default:
			usage(argv[0], EXIT_FAILURE);
			break;
		}
	}

	printf("Starting fan at %d %% duty cycle, %d Hz, on GPIO (BCM) %d\n",
	       duty, frequency, pin);

	strcpy(ip_address, "localhost");
	strcpy(ip_port, "8888");

	rc = gpioInitialise();

	if (PI_INIT_FAILED == rc) {
		fprintf(stderr, "gpioinitialise() failed: %s\n", cmdErrStr(rc));

		return EXIT_FAILURE;
	}

	rc = gpioHardwarePWM(pin, frequency, ((duty * PI_HW_PWM_RANGE) / 100));

	if (0 > rc) {
		fprintf(stderr, "hardware_PWM() failed: %s\n",
			cmdErrStr(rc));
		gpioTerminate();

		return -1;
	}

	gpioTerminate();

	return 0;
}
