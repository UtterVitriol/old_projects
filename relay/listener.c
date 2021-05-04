#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

bool setup_socket(int *sock, int port);
bool get_input(int *sock);
bool get_port(int *port);

int main(int argc, char **argv)
{
	(void)argv;

	if (argc > 1) {
		fprintf(stderr, "listener takes to arguments.\n");
		return 1;
	}

	// SIGPIPE handler for sockets
	struct sigaction brkn_pipe = {.sa_handler = SIG_IGN};
	if (sigaction(SIGPIPE, &brkn_pipe, NULL) == -1) {
		fprintf(stderr, "Error catching SIGPIPE.\n");
	};

	int port;

	if (!get_port(&port)) {
		return 1;
	}

	int sock;

	if (!setup_socket(&sock, port)) {
		return 1;
	}

	get_input(&sock);

	close(sock);
}

bool setup_socket(int *sock, int port)
{
	/* Courtesy of
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	 * Set up socket and connect to dispatcher
	 */
	struct sockaddr_in serv_addr;

	// create socket
	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "\nSocket creation error\n");
		return false;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = port;

	// convert ip
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		fprintf(stderr,
			"\nInvalid address or Address not supported \n");
		return false;
	}

	// connect to server
	if (connect(*sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
	    0) {
		fprintf(stderr, "\nConnection failed\n");
		return false;
	}

	return true;
}

bool get_input(int *sock)
{
	// read data and print from socket

	char buffer[1024] = {'\0'};

	for (;;) {

		// read socket
		read(*sock, buffer, 1024);

		// send keep alive
		if (send(*sock, "1", 1, 0) == -1) {
			return false;
		}

		printf("%s", buffer);
		fflush(stdout);

		// reset buffer
		memset(buffer, '\0', 1024);
	}

	return true;
}

bool get_port(int *port)
{
	// get port number from environment variable "RELAY"

	char *relay = NULL;
	char *end;

	// get variable strings
	relay = getenv("RELAY");

	if (!relay) {
		fprintf(stderr, "Environment variable 'RELAY' not set.\n"
				"Requires port number to be set on "
				"environment variable RELAY\n"
				"Eg. 'export RELAY=23669'\n");
		return false;
	}

	// convert to int
	*port = strtol(relay, &end, 10);

	if (*port < 1024 || *port > 65535) {
		fprintf(stderr, "Invalid environment variable 'RELAY'.\n"
				"Requires port number to be set on "
				"environment variable RELAY\n"
				"Eg. 'export RELAY=23669'\n");
		return false;
	}

	return true;
}