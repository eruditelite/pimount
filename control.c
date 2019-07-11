/*
  control.c
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

#include "control.h"

/*
  ------------------------------------------------------------------------------
  control
*/

void *
control(void *input)
{
	struct control_input *parameters;
	int listenfd = 0;
	int connfd = 0;
	struct sockaddr_in serv_addr; 
	char sendBuff[1025];
	struct control_message message;
	time_t epoch;
	struct tm *now;

	parameters = (struct control_input *)input;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(parameters->port);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

	listen(listenfd, 10); 

	for (;;) {
		int bytes_read;

		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

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
			case CONTROL_GET_TIME:
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
	}

	close(connfd);
	pthread_exit(NULL);
}
