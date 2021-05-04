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

bool setup_socket(Client *client);
void buffer_input(Client *client);
bool is_closed(int *sock);
bool get_port(int *port);

bool init_client_array(Client *client);
int add_client(Client *client, int sock);
int del_client(Client *client, int idx);
int cleanup_client(Client *client);

int send_clients(Client *client, char *msg, size_t len);

void *get_clients(void *ctx);
bool handle_args(int argc, char **argv, Client *client);
void print_help(void);
void no_buffer_input(Client *client);

void cleanup(Client *Client);

int main(int argc, char **argv)
{

	Client client;
	client.termios = false;
	client.max = 20;
	client.running = true;
	client.addrlen = sizeof(client.addr);

	if (!handle_args(argc, argv, &client)) {
		return 1;
	}

	// SIGPIPE handler for sockets
	struct sigaction brkn_pipe = {.sa_handler = SIG_IGN};
	if (sigaction(SIGPIPE, &brkn_pipe, NULL) == -1) {
		fprintf(stderr, "Error catching SIGPIPE.\n");
	};

	if (!get_port(&client.port)) {
		return 1;
	}

	if (!setup_socket(&client)) {
		return 1;
	}

	if (!init_client_array(&client)) {
		close(client.sock);
		return 1;
	}

	pthread_t thr;
	pthread_create(&thr, NULL, &get_clients, (void *)&client);

	if (client.termios) {
		no_buffer_input(&client);
	} else {
		buffer_input(&client);
	}

	if (client.termios) {
		cleanup(&client);
	}

	client.running = false;

	pthread_cancel(thr);
	pthread_join(thr, NULL);

	cleanup_client(&client);
}

bool setup_socket(Client *client)
{
	/* Courtesy of
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	 * Set up socket to listen for connections
	 */

	int opt = 1;
	int len = sizeof(len);

	// create socket
	if ((client->sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket creation failed");
		return false;
	}

	// set socket options
	if (setsockopt(client->sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		       &opt, sizeof(opt))) {
		perror("Setsockopt");
		return false;
	}

	client->addr.sin_family = AF_INET;
	client->addr.sin_addr.s_addr = INADDR_ANY;
	client->addr.sin_port = client->port;

	// bind socket to port
	if (bind(client->sock, (struct sockaddr *)&client->addr,
		 sizeof(client->addr)) < 0) {
		perror("Bind failed");
		return false;
	}

	// listen for connections
	if (listen(client->sock, 3) < 0) {
		perror("Listen failed");
		return false;
	}

	return true;
}

void buffer_input(Client *client)
{
	// get buffered input

	char *line = NULL;
	size_t linelen = 0;
	int read;

	// get input from stdin
	while ((read = getline(&line, &linelen, stdin)) >= 0) {

		// check for EOF
		if (feof(stdin)) {
			break;
		}

		send_clients(client, line, read);
	}

	// free line
	if (line) {
		free(line);
	}
}

void no_buffer_input(Client *client)
{
	// This code is mostly from the book. Pg. 228-230.
	// get non-buffered input

	int fd;

	fd = open("/dev/tty", O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		perror("/dev/tty");
		return;
	}

	client->fd = fd;

	// get origional tty settings
	tcgetattr(fd, &client->origional);

	// set new tty settings
	struct termios terminal;
	tcgetattr(fd, &terminal);
	terminal.c_lflag &= ~ICANON;
	terminal.c_cc[VMIN] = 1;
	terminal.c_cc[VTIME] = 0;
	tcsetattr(fd, TCSANOW, &terminal);

	fd_set fds;

	for (;;) {
		struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		ssize_t rlen = 0;

		// check if input
		int rv = select(fd + 1, &fds, NULL, NULL, &timeout);

		char buff[1024];

		if (rv > 0) {
			// read input
			rlen = read(fd, buff, sizeof(buff));

			// check for err or EOF
			if (rlen <= 0 || buff[0] == 4) {
				break;
			}

			buff[rlen] = '\0';

			// send input
			send_clients(client, buff, rlen);
		}
	}
}

int send_clients(Client *client, char *msg, size_t len)
{
	// send message to all clients

	for (size_t i = 0; i < client->count; i++) {
		if (send(client->clients[i], msg, len + 1, 0) == -1) {
			fprintf(stderr, "Pipe error\n");
		}

		if (is_closed(&client->clients[i])) {
			del_client(client, i);
			i--;
			continue;
		}
	}

	return 1;
}

bool is_closed(int *sock)
{
	// check socket for disconnect

	struct pollfd pfd;
	pfd.events = POLLIN | POLLHUP | POLLRDNORM;
	pfd.fd = *sock;
	pfd.revents = 0;
	char x = '\0';
	ssize_t r;

	// check for keep alive
	if (poll(&pfd, 1, 100) > 0) {
		if ((r = recv(*sock, &x, 1, MSG_DONTWAIT)) == 0 || x != '1') {
			return true;
		}
	}

	return false;
}

bool get_port(int *port)
{
	// get port number from environment variable "RELAY"

	char *relay = NULL;
	char *end;

	// get variable string
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

bool init_client_array(Client *client)
{
	// alloc client array

	client->clients = malloc(client->max * sizeof(int));

	if (!client->clients) {
		return false;
	}

	client->count = 0;

	return true;
}

int add_client(Client *client, int sock)
{
	// add client to client array

	client->clients[client->count] = sock;
	client->count++;

	return 1;
}

int del_client(Client *client, int idx)
{
	// remove client from client array

	close(client->clients[idx]);
	client->clients[idx] = client->clients[client->count - 1];
	client->count--;

	return 1;
}

void *get_clients(void *ctx)
{
	// check for new clients

	int new_socket;
	Client *client;

	client = (Client *)ctx;

	for (;;) {

		// check for end of program
		if (!client->running) {
			break;
		}

		// check for max clients
		if (client->count == client->max) {
			if (client->sock != -1) {
				close(client->sock);
				client->sock = -1;
			}
			continue;
		} else if (client->sock == -1 && client->count < client->max) {
			if (!setup_socket(client)) {
				perror("Socket");
				exit(EXIT_FAILURE);
			}
		}

		// accept new connection
		if ((new_socket =
			 accept(client->sock, (struct sockaddr *)&client->addr,
				(socklen_t *)&client->addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		add_client(client, new_socket);
	}

	return NULL;
}

int cleanup_client(Client *client)
{
	// close client connections and free client array

	for (size_t i = 0; i < client->count; i++) {
		close(client->clients[i]);
	}

	free(client->clients);

	return 1;
}

bool handle_args(int argc, char **argv, Client *client)
{
	// handles command line arguments

	int c;
	int64_t temp;
	char *end;

	while ((c = getopt(argc, argv, "l:b")) >= 0) {
		switch (c) {
		case 'l':
			temp = strtol(optarg, &end, 10);
			if (temp < 1 || temp > 100) {
				print_help();
				return false;
			}

			client->max = strtoull(optarg, &end, 10);
			break;
		case 'b':
			client->termios = true;
			break;
		default:
			print_help();
			return false;
		}
	}

	if (optind != argc) {
		printf("Invalid argument '%s'\n", argv[optind]);
		print_help();
		return false;
	}

	return true;
}

void print_help(void)
{
	// print susage statement

	fprintf(stderr, "Usage: ./out [-b] -l <num>\n"
			"Requires port number to be set on "
			"environment variable RELAY\n"
			"-b toggles line buffering\n"
			"-l Set maximum number of listeners (1 - 100)\n");
}

void cleanup(Client *client)
{
	// reset tty settings
	tcsetattr(client->fd, TCSANOW, &client->origional);
}