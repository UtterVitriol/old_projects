#include <check.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

START_TEST(test_socket_creation)
{
	struct sockaddr_in serv_addr;

	int sock; 
	int port = 6969;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "\nSocket creation error\n");
		ck_assert(false);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = port;

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		fprintf(stderr,
			"\nInvalid address or Address not supported \n");
		ck_assert(false);
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
	    0) {
		fprintf(stderr, "\nConnection failed\n");
		ck_assert(true);
	}

	ck_assert(true);
}
END_TEST

START_TEST(test_port_listener)
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

static TFun core_tests[] = {
	test_socket_creation,
	test_port_listener,
	NULL
};

Suite *suite_listener(void)
{
	Suite *s = suite_create("Listener");

	TCase *tc = tcase_create("Core");
	TFun *curr = core_tests;
	while (*curr) {
		tcase_add_test(tc, *curr++);
	}

	suite_add_tcase(s, tc);

	return s;
}
