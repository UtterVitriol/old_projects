#pragma once

#include <stdbool.h>
#include <stdlib.h>

void gen_primes(void);
bool is_prime(size_t n);
size_t next_prime(size_t N);
void *work_thread(void *n);
void *print_thread(void *n);
void restart_prime(int sig);
void skip_prime(int sig);
void reverse_prime(int sig);
int handle_args(int argc, char **argv);
void print_help(void);
void init_data(void);

union prime_vehicle {
	void *ptr;
	size_t prime;
};

typedef struct {
	union prime_vehicle p;
	bool has_reset;
	bool skip;
	bool increase;
	size_t prime_start;
	size_t prime_last;
	bool has_end;
	size_t end;
	int sleep_dur;
	bool changed;
} prime;