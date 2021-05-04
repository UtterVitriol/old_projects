#include <check.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <pthread.h>

#include <termio.h>
#include <fcntl.h>

typedef struct Client {
	int sock;
	int *clients;
	size_t count;
	size_t max;
	struct sockaddr_in addr;
	int addrlen;
	int port;
	bool running;
	bool termios;
	struct termios origional;
	int fd;
} Client;

START_TEST(test_client_creation)
{
	Client client;
	client.termios = false;
	client.max = 20;
	client.running = true;
	client.addrlen = sizeof(client.addr);
	client.port = 6969;

	/* Courtesy of
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	 * Set up socket to listen for connections
	 */

	int opt = 1;
	int len = sizeof(len);

	// create socket
	if ((client.sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket creation failed");
		ck_assert(false);
	}

	// set socket options
	if (setsockopt(client.sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		       &opt, sizeof(opt))) {
		perror("Setsockopt");
		ck_assert(false);
	}

	client.addr.sin_family = AF_INET;
	client.addr.sin_addr.s_addr = INADDR_ANY;
	client.addr.sin_port = client.port;

	// bind socket to port
	if (bind(client.sock, (struct sockaddr *)&client.addr,
		 sizeof(client.addr)) < 0) {
		perror("Bind failed");
		ck_assert(false);
	}

	// listen for connections
	if (listen(client.sock, 3) < 0) {
		perror("Listen failed");
		ck_assert(false);
	}

	ck_assert(true);

}
END_TEST

START_TEST(test_port_dispatcher)
{
	// get port number from environment variable "RELAY"

	const char *relay = "6969";
	char *end;
	long port = 0;

	if (!relay) {
		fprintf(stderr, "Environment variable 'RELAY' not set.\n");
		ck_assert(false);
	}

	// convert to int
	port = strtol(relay, &end, 10);

	if (port < 1024) {
		fprintf(stderr, "Invalid environment variable 'RELAY'.\n");
		ck_assert(false);
	}

	ck_assert(true);

}
END_TEST

START_TEST(test_client_array_creation)
{

	Client client;
	client.termios = false;
	client.max = 20;
	client.running = true;
	client.addrlen = sizeof(client.addr);
	client.port = 6969;
	
	client.clients = malloc(client.max * sizeof(int));
	if (client.clients == NULL)
		ck_assert(false);
	else
		ck_assert(true);

	free(client.clients);

}
END_TEST

START_TEST(test_client_sock_insert)
{

	Client client;
	client.termios = false;
	client.max = 20;
	client.running = true;
	client.addrlen = sizeof(client.addr);
	client.port = 6969;
	client.count = 0;

	int sock = 1337;
	
	client.clients = malloc(client.max * sizeof(int));

	

	client.clients[client.count] = sock;
	client.count++;

	ck_assert_int_eq(client.clients[0], sock);

	free(client.clients);

}
END_TEST

static TFun core_tests[] = {
	test_client_creation,
	test_port_dispatcher,
	test_client_array_creation,
	test_client_sock_insert,
	NULL
};

Suite *suite_dispatcher(void)
{
	Suite *s = suite_create("Dispatcher");

	TCase *tc = tcase_create("Core");
	TFun *curr = core_tests;
	while (*curr) {
		tcase_add_test(tc, *curr++);
	}

	suite_add_tcase(s, tc);

	return s;
}

