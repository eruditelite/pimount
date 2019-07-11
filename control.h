/*
  control.h
*/

#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h> 

struct control_input {
	unsigned short port;
};

enum control_command {
	CONTROL_GET_TIME,
	CONTROL_GET_STATUS
};

struct control_time {
	struct tm time;
};

struct control_status {
	int temperature;
};

union control_message_body {
	struct control_time time;
	struct control_status status;
};

struct control_message {
	enum control_command command;
	union control_message_body body;
};

void *control(void *);

#endif	/* _CONTROL_H_ */
