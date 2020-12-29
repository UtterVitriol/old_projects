/* wordsorter-
 *	sorts and prints words in various orders
 * 	takes variable number of files and sorts them one at a time
 * 	printing them in the order specified
 * 
 * 	Usage: ./ws [-rnlsSauih] [-c <n>] [-C <n>] [file_name(s)...]
 *	Each flag, except r, can only be input once..
 *	-c - prints first <n> number of results from sorted list
 *	-C - prints last <n> number of results from sorted list
 *	-r sorts in reverse order
 *	-n sorts by leading numbers in words
 *	-l sorts by word length
 *	-s sorts words by scrabble score
 *	-S sorts by scrabble score and removes impossible words
 *	-a sorts alphabetically
 *	-u prints only unique words
 *	-i has sorts treat words as lowercase
 *	-h prints this help message
 * 
 *	If no arguments are given, words will be retrieved one at -
 * 	a time from stdin
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

void user_input(void);

void handle_args(int, char **, int *, wchar_t **);

wchar_t **read_file(char *, int *);

void sort_alpha(wchar_t **, int *, int);

void sort_as_num(wchar_t **, int *);

void sort_length(wchar_t **, int *);

int scrabble_score(wchar_t *);

wchar_t **sort_scrabble(wchar_t **, int *, int);

void print_words(wchar_t **, int*, int, int, int);

void print_reverse(wchar_t **, int *, int, int, int);

void free_words(wchar_t **, int *);

void print_help(void);

int main(int argc, char **argv)
{
	// calls handle_args and user_input
	
	setlocale(LC_ALL, "");
	
	wchar_t **words = NULL;

	int count = 0;
	
	if(argc > 1){
		handle_args(argc, argv, &count, words);
	}else{
		user_input();
	}
	
}

void user_input(void)
{
	/* gets a list of words from stdin and prints them alphabetically */

	wchar_t temp[50] = {'\0'}, **words;
	int size = 20, count = 0, temp_size;

	// initalized initial array
	words = calloc(size, sizeof(wchar_t *));

	while(1){
		if(count > 0){
			// checks to make array bigger
			if(count == (size - 5)){
				temp_size = size;
				size *= 2;
				
				// makes array twice as big
				words = realloc(words, size * sizeof(wchar_t*));
				
				// initializes new pointers
				for(int i = temp_size - 1; i < size; i++){
					words[i] = NULL;
				}
			}

			// print current words in array
			puts("");
			puts("Words in list:");
			for(int i = 0; i < count; i ++){
				printf("%ls\n", words[i]);
			}
		}

		printf("Enter a word to be added to the sort list, \"!quit\"");
		printf(" to exit:\n-> ");

		// get user input
		fgetws(temp, 50, stdin);

		// if user didn't enter anything
		if(wcslen(temp) == 1){
			continue;
		}

		if(wcsstr(temp, L"!quit")){
			break;
		}

		// adds word to words array
		words[count] = malloc(wcslen(temp) * sizeof(wchar_t) + 10);
		wcscpy(words[count], temp);

		words[count][wcslen(words[count]) - 1] = '\0';

		count += 1;
	}
	
	if(count == 0){
		puts("You didn't enter any words!");
		exit(0);
	}
	
	// skips sorting if only one word
	if(count > 1){
		sort_alpha(words, &count, 0);
	}
	
	print_words(words, &size, count, 0, 0);
}

void handle_args(int argc, char **argv, int *count, wchar_t **words)
{
	// handles arguments, calls file i/o, sorting and printing functions

	int opt, opt_count = 0, flag = 0, r_flag = 0, i_flag = 0, S_flag = 0;
	int num = 10000, start = 0, u_flag = 0;

	char opts[10] = {'\0'}, redun[6] = "ansSl", *redun_ptr;

	while ((opt = getopt(argc, argv, "C:c:rnlsauhiS")) != -1){

		// checks if flagn in redun is in opt, replaces it
		if(strchr(redun, opt)){
			if(strchr(opts, opt)){
				print_help();
			}
			for(int i = 0; i < (int)strlen(redun); i++){
				if((redun_ptr = strchr(opts, redun[i]))\
				    != NULL){
					*redun_ptr = opt;
					flag = 1;
					break;
				}
			}
		}

		if(flag){
			flag = 0;
			continue;
		}

		// makes sure theres only one of each flag besides 'r'
		if(opt != 'r' && strchr(opts, opt)){
			print_help();
		}

		switch(opt){
		case 'c': 
			opts[opt_count] = opt;
			opt_count++;
			if(isdigit(optarg[0]) == 0){
				print_help();
			}
			num = atoi(optarg);
			break;
		case 'C':
			opts[opt_count] = opt;
			opt_count++;
			if(isdigit(optarg[0]) == 0){
				print_help();
			}
			start = atoi(optarg);
			break;
		case 'r': 
			if(!strchr(opts, 'r')){
				opts[opt_count] = opt;
				opt_count++;
			}
			if(r_flag == 0){
				r_flag =1;
			}else{
				r_flag = 0;
			}
			break; 
		case 'n': 
			opts[opt_count] = opt;
			opt_count++;
			break; 
		case 'l': 
			opts[opt_count] = opt;
			opt_count++;
			break;
		case 's': 
			opts[opt_count] = opt;
			opt_count++;
			break;
		case 'S':
			opts[opt_count] = opt;
			opt_count++;
			break;
		case 'a':
			opts[opt_count] = opt;
			opt_count++;
			break;
		case 'u': 
			opts[opt_count] = opt;
			opt_count++;
			break; 
		case 'h': 
			print_help();
			break;
		case 'i':
			opts[opt_count] = opt;
			opt_count++;
			i_flag = 1;
			break;
		case '?':
			print_help();
			exit(1);
		}
	}

	// print help if no files follow args
	if(optind == argc){
		print_help();
	}

	// respond to flags
	for(int i = optind; i < argc; i++){
		*count = 0;
		if((words = read_file(argv[i], count)) == NULL ){
			continue;
		}

		for(int j = 0; j < (int)strlen(opts); j++){
			switch(opts[j]){
			case 'n':
				sort_as_num(words, count);
				break;
			case 'l':
				sort_length(words, count);
				break;
			case 's':
				sort_scrabble(words , count, S_flag);
				break;
			case 'S':
				S_flag = 1;
				words = sort_scrabble(words , count, S_flag);
				break;
			case 'a':
				sort_alpha(words, count, i_flag);
				break;
			case 'u':
				u_flag = 1;
				break;
			}
		}

		// sorts by alpha if no flags are given
		if(strlen(opts) == 0){
			sort_alpha(words, count, i_flag);
		}

		// print words
		if(r_flag){
			print_reverse(words, count, num, start, u_flag);
		}else{
			print_words(words, count, num, start, u_flag);
		}

		puts("");
	}

	return;
}

wchar_t **read_file(char *file, int *count)
{
	// gets words from files

	FILE *fp;
	wchar_t word[50], c, **words;
	int word_count = 1;
	
	fp = fopen(file, "r");
	
	if(!fp){
		printf("File: \"%s\", does not exist...\n\n", file);
		return(NULL);
	}

	// gets word count from file
	while((c = fgetwc(fp)) != EOF){
		if(c == L' ' || c == L'\n' || c == L'\t'){
			word_count++;
		}
	}

	rewind(fp);

	// gets wrods from file
	words = malloc(word_count * sizeof(wchar_t *));
	while(fwscanf(fp, L"%ls", word) == 1){
		words[*count] = malloc(50 * sizeof(wchar_t) + 10);
		wcscpy(words[*count], word);
		*count += 1;
	}

	fclose(fp);
	return(words);
}

void to_lower(wchar_t *lower_one, wchar_t *lower_two, wchar_t *one,\
	      wchar_t *two)
{
	// makes chars lowercase for -i

	for(int i = 0; i < (int)wcslen(one); i++){
		lower_one[i] = towlower(one[i]);
	}
	
	for(int i = 0; i < (int)wcslen(two); i++){
		lower_two[i] = towlower(two[i]);
	}
}

void sort_alpha(wchar_t **words, int *count, int i_flag)
{
	// sorts words alphabetically

	wchar_t *temp = NULL, lower_one[50] = {'\0'}, lower_two[50] = {'\0'};

	// sorts as if words were lowercase
	if(i_flag){
		for(int i = 0; i < *count; i++){
			for(int j = 0; j < *count; j++){
				to_lower(lower_one, lower_two,\
					 words[i], words[j]);
				if(wcscmp(lower_one, lower_two) < 0){
					temp = words[j];
					words[j] = words[i];
					words[i] = temp;
				}
			}
		}
	}else{
		for(int i = 0; i < *count; i++){
			for(int j = 0; j < *count; j++){
				if(wcscmp(words[i], words[j]) < 0){
					temp = words[j];
					words[j] = words[i];
					words[i] = temp;
				}
			}
		}
	}

	return;
}

void sort_as_num(wchar_t **words, int *count)
{
	// sorts words based on leading digits

	wchar_t *temp, *ptr1, *ptr2;
	long ret1, ret2;
	for(int i = 0; i < *count; i++){
		ret1 = wcstol(words[i], &ptr1, 10);
		for(int j = 0; j < *count; j++){
			 ret2 = wcstol(words[j], &ptr2, 10);
			if(ret1 < ret2){
				temp = words[j];
				words[j] = words[i];
				words[i] = temp;
			}
		}
	}

	return;
}

void sort_length(wchar_t **words, int *count)
{
	// sorts words based on length

	wchar_t *temp;

	for(int i = 0; i < *count; i++){
		for(int j = 0; j < *count; j++){
			if(wcslen(words[i]) < wcslen(words[j])){
				temp = words[j];
				words[j] = words[i];
				words[i] = temp;
			}
		}
	}

	return;
}


wchar_t **sort_scrabble(wchar_t **words, int *count, int S_flag)
{
	// sorts words based on their scrabble score

	int temp = 0, has_int = 0;
	wchar_t **temp_words;

	// creates new word list, removing impossible scrabble words
	if(S_flag){
		
		temp_words = calloc(*count, sizeof(wchar_t **));
		
		for(int i = 0; i < *count; i++){
			for(int j = 0; j < (int)wcslen(words[i]); j++){
				if(isalpha(words[i][j]) == 0) {
					has_int = 1;
					break;
				}
			}
			if(!has_int){
				temp_words[temp] = malloc(wcslen(words[i]) *\
							  sizeof(wchar_t) + 10);
				wcscpy(temp_words[temp], words[i]);
				temp++;
			}else{
				has_int = 0;
			}
		}
		sort_scrabble(temp_words, &temp, 0);
		free_words(words, count);
		if(temp == 0){
			free_words(temp_words, &temp);
			puts("No valid words to sort");
			exit(0);
		}
		*count = temp;
		return(temp_words);
	}

	// sorts the words based on scrabble score
	wchar_t *temp_two;
	for(int i = 0; i < *count; i++){
		for(int j = 0; j < *count; j++){
			if(scrabble_score(words[i]) > scrabble_score(words[j])){
				temp_two = words[j];
				words[j] = words[i];
				words[i] = temp_two;
			}
		}
	}

	return 0;
}

int scrabble_score(wchar_t *word)
{
	// evaluations scrabble score for a word

	wchar_t one[] = L"aeoiulnst";
	wchar_t two[] = L"dg";
	wchar_t three[] = L"bcmp";
	wchar_t four[] = L"fhvwy";
	wchar_t five[] = L"k";
	wchar_t eight[] = L"jx";
	wchar_t ten[] = L"qz";

	int score = 0;

	for(int i = 0; i < (int)wcslen(word); i++){
		if(wcschr(one, towlower(word[i])))
			score += 1;
		if(wcschr(two, towlower(word[i])))
			score += 2;
		if(wcschr(three, towlower(word[i])))
			score += 3;
		if(wcschr(four, towlower(word[i])))
			score += 4;
		if(wcschr(five, towlower(word[i])))
			score += 5;
		if(wcschr(eight, towlower(word[i])))
			score += 8;
		if(wcschr(ten, towlower(word[i])))
			score += 10;
	}

	return(score);
}

void print_words(wchar_t **words, int *count, int num, int start, int u_flag)
{
	// prints words

	int temp_count = *count, i = 0, temp = 0;
	
	if(num < *count){
		*count = num;
	}
	
	
	if(start > 0){
		i = (*count) - start;
		
	}

	// creates new word list with only unique words and prints them
	if(u_flag){
		int is_in = 0, u_i = 0;
		wchar_t **temp_words = NULL;
		temp_words = calloc(temp_count, sizeof(wchar_t *));
		temp_words[temp] = malloc(wcslen(words[0]) * sizeof(wchar_t)\
					  + 10);
		wcscpy(temp_words[temp], words[0]);
		temp++;
		
		for(int i = 1; i < temp_count; i++){
			for(int j = 0; j < temp; j++){
				if(wcscmp(words[i], temp_words[j]) == 0){
					is_in = 1;
					break;
				}
			}
			if(!is_in){
				temp_words[temp] = malloc(wcslen(words[i]) *\
							  sizeof(wchar_t) + 10);
				wcscpy(temp_words[temp], words[i]);  
				temp++;
			}else{
				is_in = 0;
			}
		}

		if(num < temp){
			temp = num;
		}
		if(start > 0){
			if(start <= temp){
				u_i = temp - start;
			}
		}

		for(; u_i < temp; u_i++){
			printf("%ls\n", temp_words[u_i]);
		}
		free_words(temp_words, &temp_count);
	}else{
		// prints words as they are
		for(; i < *count; i ++){
			printf("%ls\n", words[i]);
		}
	}

	free_words(words, &temp_count);
}

void print_reverse(wchar_t **words, int *count, int num, int start, int u_flag)
{
	// prints words in reverse order

	int temp_count = *count, temp = 0, utemp = 0;

	if(num < *count){
		temp = *count - num;
	}

	if(start > 0 && start <= num){
		*count = temp + start;
	}

	// creates new word list with only unique words and prints them reversed
	if(u_flag){
		int is_in = 0, end = 0;
		wchar_t **temp_words;
		temp_words = calloc(temp_count, sizeof(wchar_t *));
		temp_words[utemp] = malloc(wcslen(words[0]) * sizeof(wchar_t)\
						  + 10);
		wcscpy(temp_words[utemp], words[0]);
		utemp++;
		
		for(int i = 1; i < temp_count; i++){
			for(int j = 0; j < utemp; j++){
				if(wcscmp(words[i], temp_words[j]) == 0){
					is_in = 1;
					break;
				}
			}
			if(!is_in){
				temp_words[utemp] = malloc(wcslen(words[i]) *\
							   sizeof(wchar_t) + 9);
				wcscpy(temp_words[utemp], words[i]);  
				utemp++;
			}else{
				is_in = 0;
			}
		}
	
		if(num < utemp){
			end = utemp - num;
		}
		
		if(start > 0){
			if((end + start) < utemp){
				utemp = end + start;
			}
		}

		for(int i = utemp - 1; i >= end; i--){
			printf("%ls\n", temp_words[i]);
		}
		free_words(temp_words, &temp_count);

	}else{
		// prints words in reversed order as they are
		for(int i = *count - 1; i >= temp ; i--){
			printf("%ls\n", words[i]);
		}
	}

	*count = 0;
	free_words(words, &temp_count);
}

void print_help(void)
{
	// prints usage statement and exits program

	puts("Usage: ./ws [-rnlsSauih] [-c <n>] [-C <n>] [file_name(s)...]");
	puts("");
	puts("Each flag, except r, can only be input once..");
	puts("");
	puts("-c - prints first <n> number of results from sorted list");
	puts("-C - prints last <n> number of results from sorted list");
	puts("-r sorts in reverse order");
	puts("-n sorts by leading numbers in words");
	puts("-l sorts by word length");
	puts("-s sorts words by scrabble score");
	puts("-S sorts by scrabble score and removes impossible words");
	puts("-a sorts alphabetically");
	puts("-u prints only unique words");
	puts("-i has sorts treat words as lowercase");
	puts("-h prints this help message");
	puts("");
	puts("If no arguments are given, words will be retrieved -");
	puts("one at a time from stdin");

	exit(0);
}

void free_words(wchar_t **words, int *count)
{
	// frees words array

	for(int i = 0; i < *count; i++){
		free(words[i]);
		words[i] = NULL;
	}

	free(words);
	words = NULL;
}