/*
  control.c

  Excersize the "control" interface of pimount.
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "../server.h"

/*
  ------------------------------------------------------------------------------
  main
*/

int
main(int argc, char *argv[])
{
	int sockfd = 0;
	int n = 0;
	struct sockaddr_in serv_addr;
	struct server_message message;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ip of server> <port>\n", argv[0]);

		return 1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));

		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) != 1) {
		fprintf(stderr, "inet_pton() failed\n");

		return 1;
	}

	/*
	  Get the Time (SERVER_GET_TIME)
	*/

	if (connect(sockfd,
		    (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "connect() failed: %s\n", strerror(errno));

		return 1;
	}

	message.command = SERVER_GET_TIME;

	if (-1 == send(sockfd, &message, sizeof(message), 0)) {
		fprintf(stderr, "send() failed: %s\n", strerror(errno));
	} else {
		n = read(sockfd, &message, sizeof(message));

		if (0 != n)
			printf("%s", asctime(&message.body.time.time));
	}

	/*
	  Get the Status (SERVER_GET_STATUS)
	*/

#if 0
	message.command = SERVER_GET_STATUS;
	send(sockfd, &message, sizeof(message), 0);
	n = read(sockfd, &message, sizeof(message));

	if (0 != n)
		printf("temperature = %d\n", message.body.status.temperature);
#endif

	return 0;
}
