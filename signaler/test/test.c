#include <check.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../signaler_utils.h"

prime data;

START_TEST(test_next_prime)
{
	// test next_prime function

	init_data();
	
	size_t next;

	next = next_prime(data.p.prime);
	ck_assert(next == 3);

	next = next_prime(next);
	ck_assert(next == 5);
}
END_TEST

START_TEST(test_sighup)
{
	// test sighup handling

	init_data();

	ck_assert(data.has_reset == false);

	struct sigaction restart = { .sa_handler = restart_prime };
	if(sigaction(SIGHUP, &restart, NULL) == -1){
		puts("Error catching SIGHUP.");
	};

	raise(SIGHUP);

	ck_assert(data.has_reset == true);
}
END_TEST

START_TEST(test_sigusr1)
{
	// test sigusr1 handling

	init_data();

	ck_assert(data.skip == false);

	struct sigaction skip = { .sa_handler = skip_prime };
	if(sigaction(SIGUSR1, &skip, NULL) == -1){
		puts("Error catching SIGUSR1.");
	};

	raise(SIGUSR1);

	ck_assert(data.skip == true);
}
END_TEST

START_TEST(test_sigusr2)
{
	// test sigusr2 handling

	init_data();

	ck_assert(data.increase == true);

	struct sigaction reverse = { .sa_handler = reverse_prime };
	if(sigaction(SIGUSR2, &reverse, NULL) == -1){
		puts("Error catching SIGUSR2.");
	};

	raise(SIGUSR2);

	ck_assert(data.skip == false);

	raise(SIGUSR2);

	ck_assert(data.increase == true);
}
END_TEST

static TFun core_tests[] = {
	test_next_prime,
	test_sighup,
	test_sigusr1,
	test_sigusr2,
	NULL
};


Suite *suite_signaler(void)
{
	Suite *s = suite_create("Signaler");

	TCase *tc = tcase_create("Core");
	TFun *curr = core_tests;
	while(*curr){
		tcase_add_test(tc, *curr++);
	}

	suite_add_tcase(s, tc);

	return s;
}