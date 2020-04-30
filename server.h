/*
  server.h
*/

#ifndef _SERVER_H_
#define _SERVER_H_

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

struct server_input {
	unsigned short port;
};

enum server_command {
	SERVER_GET_TIME,
	SERVER_GET_STATUS
};

struct server_time {
	struct tm time;
};

struct server_status {
	int temperature;
};

union server_message_body {
	struct server_time time;
	struct server_status status;
};

struct server_message {
	enum server_command command;
	union server_message_body body;
};

void *server(void *);

#endif	/* _SERVER_H_ */
