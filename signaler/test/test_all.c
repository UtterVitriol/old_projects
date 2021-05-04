#include <check.h>

extern Suite *suite_signaler(void);

int main(void)
{
	SRunner *sr = srunner_create(NULL);
	
	srunner_add_suite(sr, suite_signaler());

	srunner_run_all(sr, CK_NORMAL);

	int failed = srunner_ntests_failed(sr);

	srunner_free(sr);

	return !!failed;
}