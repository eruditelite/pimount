/*
  server.c
*/

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

#include "server.h"

/*
  ------------------------------------------------------------------------------
  server
*/

void *
server(void *input)
{
	struct server_input *parameters;
	int listenfd = 0;
	int connfd = 0;
	struct sockaddr_in serv_addr; 
	char sendBuff[1025];
	struct server_message message;
	time_t epoch;
	struct tm *now;
	pthread_t this;
	struct sched_param params;
	socklen_t addr_len;

	parameters = (struct server_input *)input;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (-1 == listenfd) {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; /* All interfaces... */
	serv_addr.sin_port = htons(parameters->port);

	if (-1 == bind(listenfd,
		       (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
		fprintf(stderr, "bind() failed: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	addr_len = sizeof(serv_addr);

	/* Display the port number. */
	if (-1 ==
	    getsockname(listenfd, (struct sockaddr *)&serv_addr, &addr_len)) {
		fprintf(stderr, "getsockname() failed: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	printf("Listening on port %d\n", ntohs(serv_addr.sin_port));

	if (-1 == listen(listenfd, 10)) {
		fprintf(stderr, "listen() failed: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	/* Run at a High Priority -- Higher than the Server Thread */
	this = pthread_self();
	params.sched_priority = 50;

	if (0 != pthread_setschedparam(this, SCHED_RR, &params))
		fprintf(stderr, "pthread_setschedparam() failed!\n");

	for (;;) {
		int bytes_read;

		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

		if (-1 == connfd) {
			fprintf(stderr, "listen() failed: %s\n", strerror(errno));
			pthread_exit(NULL);
		}

		for (;;) {
			pthread_testcancel();
			bytes_read = read(connfd, &message, sizeof(message));

			/*
			 * If read returns 0, the client closed the
			 * connection, so break and wait for another
			 * connection.
			 */

			if (0 == bytes_read)
				break;

			switch (message.command) {
			case SERVER_GET_TIME:
				epoch = time(NULL);
				now = localtime(&epoch);
				memcpy(&message.body.time, now,
				       sizeof(struct tm));
				write(connfd, &message, sizeof(message));
				break;
			default:
				fprintf(stderr,
					"%s:%d - Unknown Command: %d\n",
					__FILE__, __LINE__, message.command);
				break;
			}
		}

		close(connfd);
	}

	pthread_exit(NULL);
}
