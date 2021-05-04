/*
 * Calculates and prints prime numbers with a delay.
 * Starts to significantly slow down around 17 digits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

#include "signaler_utils.h"

// global for signal handlers to work with
prime data;

int main(int argc, char **argv)
{

	// sighup handler
	struct sigaction restart = {.sa_handler = restart_prime};
	if (sigaction(SIGHUP, &restart, NULL) == -1) {
		fprintf(stderr, "Error catching SIGHUP.\n");
	};

	// sigusr1 handler
	struct sigaction skip = {.sa_handler = skip_prime};
	if (sigaction(SIGUSR1, &skip, NULL) == -1) {
		fprintf(stderr, "Error catching SIGUSR1.\n");
	};

	// sigusr2 handler
	struct sigaction reverse = {.sa_handler = reverse_prime};
	if (sigaction(SIGUSR2, &reverse, NULL) == -1) {
		fprintf(stderr, "Error catching SIGUSR2.\n");
	};

	// init variables
	init_data();	

	// do the args
	if (handle_args(argc, argv)) {
		return 1;
	}

	data.p.prime = data.prime_start;

	gen_primes();

	return 0;
}
