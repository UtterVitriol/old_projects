#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "signaler_utils.h"

// global for signal handlers to work with
extern prime data;
pthread_t tprime;
pthread_t tprint;

void gen_primes(void)
{
	// for -s, if prime, get next else skip to loop
	if (!is_prime(data.p.prime)) {
		data.skip = true;
	} else if (data.p.prime > 2) {
		data.p.prime++;
		pthread_create(&tprime, NULL, &work_thread, &data.p.prime);
		pthread_join(tprime, &data.p.ptr);
	}

	for (;;) {
		if (!data.skip) {
			// thread to print prime
			pthread_create(&tprint, NULL, &print_thread,
				       &data.p.ptr);
		}

		// check for 2 and decreasing
		if (data.p.prime == 2 && data.increase == false) {
			pthread_join(tprint, NULL);
			break;
		}

		// thread to get next prime
		pthread_create(&tprime, NULL, &work_thread, &data.p.prime);

		if (data.skip) {
			data.skip = false;
		} else {
			// wait for print thread
			pthread_join(tprint, NULL);
		}


		if (!data.changed) {
			// wait for prime thread
			data.prime_last = data.p.prime;
			pthread_join(tprime, &data.p.ptr);
		} else {
			data.p.prime = next_prime(data.p.prime);
			data.changed = false;
		}
		
		// check if prime is past end number
		if (data.has_end) {
			if (data.increase) {
				if (data.p.prime > data.end) {
					break;
				}
			} else {
				if (data.p.prime < data.end) {
					break;
				}
			}
		}

		// check to reset prime
		if (data.has_reset) {
			data.p.prime = data.prime_start;
			data.has_reset = false;
		}
	}
}

void reverse_prime(int sig)
{
	// thread function to reverse prime order

	(void)sig;

	if (data.increase) {
		data.increase = false;
	} else {
		data.increase = true;
	}

	data.p.prime = next_prime(data.prime_last);
	data.changed = true;
}

void skip_prime(int sig)
{
	// thread frunction to set prime skip flag

	(void)sig;
	data.skip = true;
	data.p.prime = next_prime(data.p.prime);
}

void restart_prime(int sig)
{
	// thread function to set reset prime flag

	(void)sig;
	data.has_reset = true;
}

void *print_thread(void *n)
{
	// thread frunciton to print prime

	sleep(data.sleep_dur);
	printf("%lu\n", *(size_t *)n);
	return 0;
}

void *work_thread(void *n)
{
	// thread function to get next prime

	union prime_vehicle p;
	p.prime = next_prime(*(size_t *)n);
	return p.ptr;
}

bool is_prime(size_t n)
{
	// Courtesy of -
	// https://www.geeksforgeeks.org/program-to-find-the-next-prime-number/

	// checks if number is prime

	if (n <= 1)
		return false;
	if (n <= 3)
		return true;
	if (n % 2 == 0 || n % 3 == 0)
		return false;

	for (size_t i = 5; i * i <= n; i = i + 6)
		if (n % i == 0 || n % (i + 2) == 0)
			return false;

	return true;
}

size_t next_prime(size_t N)
{
	// Courtesy of -
	// https://www.geeksforgeeks.org/program-to-find-the-next-prime-number/

	// gets next prime

	if (N <= 1)
		return 2;

	size_t prime = N;
	bool found = false;

	while (!found) {
		if (data.increase) {
			prime++;
		} else {
			prime--;
		}

		if (is_prime(prime))
			found = true;
	}

	return prime;
}

int handle_args(int argc, char **argv)
{
	// handles command line arguments

	int c;
	int64_t temp;
	char *end;

	while ((c = getopt(argc, argv, "s:re:t:")) >= 0) {
		switch (c) {
		case 's':
			temp = strtol(optarg, &end, 10);
			if (temp < 1) {
				print_help();
				return 1;
			}

			data.prime_start = strtoull(optarg, &end, 10);
			break;
		case 'r':
			data.increase = false;
			break;
		case 'e':
			temp = strtol(optarg, &end, 10);
			if (temp < 1) {
				print_help();
				return 1;
			}

			data.prime_start = strtoull(optarg, &end, 10);
			break;
			data.has_end = true;
			data.end = strtol(optarg, &end, 10);
			break;
		case 't':
			temp = strtol(optarg, &end, 10);
			if (temp < 1) {
				print_help();
				return 1;
			}
			data.sleep_dur = strtol(optarg, &end, 10);
			break;
		default:
			print_help();
			return 1;
		}
	}
	return 0;
}

void print_help(void)
{
	// print susage statement

	fprintf(stderr, "Usage: ./signaler [-r] -s <num> -e <num> -t <num>\n"
			"-s set starting number (greater than 0)\n"
			"-r start in reverse mode (greater than 0)\n"
			"-e specify end number\n"
			"-t specify sleep time (positive integer value)\n");
}

void init_data(void)
{
	// initialize data

	data.prime_start = 2;
	data.has_reset = false;
	data.skip = false;
	data.increase = true;
	data.sleep_dur = 1;
	data.changed = false;
	data.p.prime = data.prime_start;
	data.prime_last = data.prime_start;
	data.has_end = false;
	data.end = 2;
}