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

#include "../control.h"

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
	struct control_message message;

	if (argc != 2) {
		printf("\n Usage: %s <ip of server> \n",argv[0]);

		return 1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Error : Could not create socket \n");

		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8888);

	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
		printf("\n inet_pton error occured\n");

		return 1;
	}

	/*
	  Get the Time (CONTROL_GET_TIME)
	*/

	if (connect(sockfd,
		    (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\n Error : Connect Failed \n");

		return 1;
	}

	message.command = CONTROL_GET_TIME;
	send(sockfd, &message, sizeof(message), 0);
	n = read(sockfd, &message, sizeof(message));

	if (0 != n)
		printf("%s", asctime(&message.body.time.time));

	/*
	  Get the Status (CONTROL_GET_STATUS)
	*/

	message.command = CONTROL_GET_STATUS;
	send(sockfd, &message, sizeof(message), 0);
	n = read(sockfd, &message, sizeof(message));

	if (0 != n)
		printf("temperature = %d\n", message.body.status.temperature);

	return 0;
}
