#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void times_table(int, int);
int str_is_int(char string[]);
void print_int_or_hex(int margin, int number, const char *flourish);
int get_args(int argc, char **argv, int *num, int *max);

int int_or_hex = 1;  // flag for printing dec or hex

int main(int argc, char **argv)
{
	// calls get_args and times_table

	int num = 0;  // base number for times table
	int max = 0;  // sets max if no args
	int error;  // used for for input validation
	
	error = get_args(argc, argv, &num, &max);
	if(!error)
		times_table(num, max);
	
	return 0;
}


int get_args(int argc, char **argv, int *temp_num, int *temp_max)
{
	
	// input validation for times tables
	
	int num = 10;  // base number for times table
	int max = num;  // sets max if no args
	int error = 0;  // used for invalid input

	for(int i = 0; i < argc; ++i){
		if(argc == 1)
			break;
		switch(i){
			case 1:
				if(!strcmp(argv[i], "-h")){
					int_or_hex = 0;
					break;
				}else{
					error = str_is_int(argv[i]);
					num = atoi(argv[i]);
					max = num;
					break;
				}
			case 2:
				if(int_or_hex == 1){
					if(error != 1)
						error = str_is_int(argv[i]);
					max = atoi(argv[i]);
					break;
				}else{
					error = str_is_int(argv[i]);
					num = atoi(argv[i]);
					max = num;
					break;
				}
			case 3:
				if(error != 1)
					error = str_is_int(argv[i]);
				max = atoi(argv[i]);
				break;
		}
	}
	
	
	if(argc == 4 && !str_is_int(argv[1]))  // no -h , but > 2 args
		error = 1;
	
	if(num > 100 || num < 0 || max > 100 || max < 0 || argc > 4)
		error = 1;

	if(error){
		puts("Usage: ./timestable [-h] [[<min(1-100)>]<max(1-100)>]");
		puts("Ex:");
		puts("\t./timestable");
		puts("\t./timestable 23");
		puts("\t./timestable 5 2");
		puts("\t./timestable -h");
		puts("\t./timestable -h 2 5");
		return 1;
	}

	*temp_num = num;
	*temp_max = max;

	return 0;
}


void times_table(int num, int max)
{
	// takes two ints and prints a times table

	int top;  // used for the horizontal flourish line	
	char temp[5];  // used for setting the margins
	sprintf(temp, "%d", num * max);  // converts int to str
	int marg = strlen(temp) + 1;  // gets # of digits in number + 1
	const char flourish[] = {"\xE2\x94\x82"};

	printf("%*c %s", marg, '*', flourish);  // * and vertical line

	for(top = 1; top <= max; top++){  // first line
		print_int_or_hex(marg,  top, "");
	}

	puts("");  // new line

	for(int j = 1; j <= top * marg + marg; j++){  // main horizontal line
		printf("%s", "\xE2\x94\x80");
	}

	puts("");  // new line

	for(int i = 1; i <= num; i++){  // handles rows
		print_int_or_hex(marg, i, flourish);
		for(int j = 1; j <= max; j++){  // handles columns
			print_int_or_hex(marg, j * i, "");
		}

		puts("");  // new line
	}
}


int str_is_int(char string[])
{
	// takes string, returns 1 if not all ints, 0 if all int

	int result = 0;
	for(int i = 0; i < (int)strlen(string); i++){
		if(string[i] == '#' || string[i] < '0' || string[i] > '9'){
			result = 1;
			break;
		}
	}

	return result;
}


void print_int_or_hex(int margin, int number, const char *flourish)
{
	// prints ints in digits or hex
	
	switch(int_or_hex){
		case 1:
			if(strlen(flourish) == 0){
				printf("%*d", margin, number);
				break;
			}
			printf("%*d %s", margin, number, flourish);
			break;
			
		case 0:
			if(strlen(flourish) == 0){
				printf("%*x", margin, number);
				break;
			}
			printf("%*x %s", margin, number, flourish);
		}
}
