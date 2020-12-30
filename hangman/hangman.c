/* hangman game
 * 
 *  create ~/.words file and place single word per line --
 *  or provide a file on the cmd line to have the words chosen from there.
 * 
 * ex: ./hangman (defaults to ~/.words file)
 * ex: ./hangman words_file (picks words from words_file)
 * 
 * Words in the word file must be between 2 and 34 characters long.
 * 
 * Punctuation in words will be show automatically and won't need to --
 * be guess.
 * 
 * Supports UTF-8 encoded characters.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

void get_stats(int *);

void save_stats(int *);

void get_word(wchar_t *, char *);

int get_input(wchar_t *);

int check_word(wchar_t *);

int game_loop(wchar_t *, int *);

int check_guess(wchar_t *, wchar_t *, wchar_t *);

void draw_man(char *, char *, char *, int *);

int main(int argc, char **argv)
{
	/* checks args, calls get_stats, get_word, game_loop and save_stats
	 */

	setlocale(LC_ALL, "");

	int stats[9] = {0};
	wchar_t word[36] = {'\0'};

	char *file_name, no_arg[] = "NULL";

	if(argc > 2){
		puts("Usage: ./hangman [wordfile]");
		exit(1);
	}

	if(argc == 2){
		file_name = argv[1];
	}else{
		file_name = no_arg;
	}

	get_stats(stats);
	get_word(word, file_name);
	game_loop(word, stats);
	save_stats(stats);
}

void draw_man(char *head, char *body, char *legs, int *wrong_guesses)
{
	// draw man for hangman

	switch(*wrong_guesses){
	case(1):
		head[1] = 'o';
		break;
	case(2):
		body[1] = '|';
		break;
	case(3):
		body[0] = '/';
		break;
	case(4):
		body[2] = '\\';
		break;
	case(5):
		legs[0] = '/';
		break;
	case(6):
		legs[2] = '\\';
		break;
	}
}

int game_loop(wchar_t *word, int *stats)
{
	// handles the main game

	time_t start, end;
	time(&start);  // gets the start time

	int h, m, s = stats[6];

	// get time from seconds
	h = (s/3600);
	m = (s - (h*3600)) / 60;
	s = (s - (h*3600))-(m*60);

	stats[0] += 1;  // increments games playd

	int size = wcslen(word), i = 0, wrong_guesses = 0;
	word[size - 1] = '\0';
	wchar_t *out_word, guess[3] = {'\0'};
	out_word = malloc(size * sizeof(wchar_t));

	// automatically print punctuation
	for(; i < size - 1; i++){
		if(iswpunct(word[i])){
			out_word[i] = word[i];
		}else{
			out_word[i] = '_';
		}
	}

	// makes sure theres a terminator and the same index of the word
	out_word[i] = L'\0';

	printf("Game %d. %d Wins/%d Losses. Average score: %.2f\n",\
	       stats[0], stats[1], stats[2], ((float)stats[3] / stats[0]));

	printf("Longest Winning Streak: %d | Longest Losing Streak: %d\n",\
	       stats[4], stats[5]);

	printf("Total time played: %02d:%02d:%02d\n", h, m, s);

	// inits the hangman body for spacing
	char head[4] = "   ", body[4] = "   ", legs[4] = "   ";

	while(1){
		if(wrong_guesses < 6){
			printf("%s %ls\n%s %d\n%s ", head, out_word,\
			      body, wrong_guesses, legs);

			if(get_input(guess)){
				free(out_word);
				exit(0);
			}

			// checks for already guessed letters
			if(wcschr(out_word, guess[0]) ||\
			   wcschr(out_word, towupper(guess[0])) ||\
			   wcschr(out_word, towlower(guess[0]))){
				printf("You already guessed \"%lc\"...\n",\
				       guess[0]);
				wrong_guesses++;
				draw_man(head, body, legs, &wrong_guesses);
				continue;
			}

			// checks for more than one letter
			if(guess[1] != L'\n'){
				puts("Invalid guess...");
				continue;
			}

			// checks if the guessed letter is in the word
			if(!check_guess(word, out_word, guess)){
				wrong_guesses++;
				draw_man(head, body, legs, &wrong_guesses);
			}

			// checks for win condition
			if(!wcscmp(word, out_word)){
				printf("    %ls\n", word);
				printf("You win! You had %d misses...\n",\
				       wrong_guesses);
				stats[1] += 1;
				stats[3] += wrong_guesses;
				stats[7]++;
				stats[8] = 0;
				if(stats[4] < stats[7]){
					stats[4] = stats[7];
				}
				free(out_word);
				break;
			}

		}else{  // handles losing
			puts("");
			draw_man(head, body, legs, &wrong_guesses);
			printf("%s %ls\n%s %d\n%s ", head, word,\
			       body, wrong_guesses, legs);
			puts("You lose!");
			stats[2] += 1;
			stats[3] += wrong_guesses;
			stats[8]++;
			stats[7] = 0;
			if(stats[5] < stats[8]){
				stats[5] = stats[8];
			}
			free(out_word);
			break;
		}
	}

	time(&end); // gets the end time
	stats[6] += (end - start); // adds time spent playing to total time

	return 1;
}

int check_guess(wchar_t *word, wchar_t *out_word, wchar_t *guess)
{
	/*checks guess agains word indexes and makes correct guess appear
	 * on out_word
	 */

	int check = 0;
	for(int i = 0; i < (int)wcslen(word); i++){
		if(word[i] == guess[0] ||\
		   word[i] == (wchar_t)towupper(guess[0]) ||\
		   word[i] == (wchar_t)towlower(guess[0])){ 
			out_word[i] = word[i];
			check++;
		}
	}

	if(check == 0){
		return 0;
	}else{
		return 1;
	}
}

int get_input(wchar_t *guess)
{
	// gets input and draings stdin

	wchar_t drain[10] = {'\0'};
	
	if(fgetws(guess, 3, stdin) == NULL){
		return 1;
	}
	
	if(!wcschr(guess, L'\n')){
		while(!wcschr(drain, L'\n')){
			if(fgetws(drain, 10, stdin) == NULL){
				return 1;
			}
		}
	}

	return 0;
}

int check_word(wchar_t *word)
{
	// checks words in file for validity

	if(wcslen(word) > 35){
		return 0;
	}

	int count = 0;
	
	if(word[0] == '\n'){
		return 0;
	}
	
	for(int i = 0; i < (int)wcslen(word) - 1; i++){
		if(iswalpha(word[i])){
			count++;
		}
		if((iswalpha(word[i]) || iswpunct(word[i]))){
			;
		}else{
			return 0;
		}
	}
	
	if(count == 0){
		return 0;
	}

	return 1;
}

void save_stats(int *stats)
{
	// saves stats to file

	FILE *stats_file;

	char file[80] = {'\0'};

	strcat(file, getenv("HOME"));
	strcat(file, "/.hangman");

	stats_file = fopen(file, "w");

	if(stats_file){
		fprintf(stats_file, "%d %d %d %d %d %d %d %d %d",\
		        stats[0], stats[1], stats[2], stats[3], stats[4],\
		        stats[5], stats[6], stats[7], stats[8]);
		fclose(stats_file);
	}else{
		puts("Error saving stats");
		exit(1);
	}
	
}

void get_word(wchar_t *word, char *file_name)
{
	// gets random valid word from a word file

	char file[80] = {'\0'};

	// checks if there was a file passed as an argument
	if(strcmp(file_name, "NULL")){
		strcpy(file, file_name);
	}else{
		strcat(file, getenv("HOME"));
		strcat(file, "/.words");
	}
	
	FILE *word_file;
	
	word_file = fopen(file, "r+");

	// checks if file exists
	if(!word_file){
		printf("File: %s, does not exist...\n", file);
		exit(1);
	}

	int *lines, line = 0, good_word_count = 0, line_to_get, lcount = 0;
	wchar_t temp[37], c = '\0';
	
	// gets count of lines
	while(c != EOF){
		c = fgetwc(word_file);
		if(c == '\n')
			lcount++;
	}

	rewind(word_file);

	// mallocs int array of size equal to number of lines in word file
	lines = malloc(sizeof(int) * lcount);
	
	/* loops through words, checking validity and adds valid indexes to 
	 * lines
	 */
	while(fgetws(temp, 37, word_file) != NULL){
		if(check_word(temp)){
			lines[good_word_count] = line;
			good_word_count++;
		}
		line++;		
	}

	// no words in file
	if(line == 0){
		puts("No words in words file...");
		free(lines);
		exit(1);
	}
	
	rewind(word_file);

	// no valid words in file
	if(good_word_count == 0){
		puts("No valid entries in words file...");
		free(lines);
		exit(1);
	}

	time_t t;
	srand((unsigned) time(&t));

	// picks random word from file unless there is only one valid word
	if(good_word_count > 1){
		line_to_get = rand() % good_word_count;
	}else{
		line_to_get = 0;
	}

	wchar_t temp_word[37] = {'\0'};

	// gets random word from the file
	for(int i = 0; i <= line - 1; i++){
		fgetws(temp_word, 37, word_file);
		if(i == lines[line_to_get]){
			break;
		}
	}

	wcscpy(word, temp_word);
	free(lines);
	fclose(word_file);
}

void get_stats(int *stats)
{
	// reads stats from ~/.hangman file

	FILE *stats_file;

	char file[80] = {'\0'};

	strcat(file, getenv("HOME"));
	strcat(file, "/.hangman");

	stats_file = fopen(file, "r");

	/* if file exists and there is data in it, else set default values
	 * and/or create file
	 */
	if(stats_file && fscanf(stats_file, "%d %d %d %d %d %d %d %d %d",\
		                &stats[0], &stats[1], &stats[2],\
		                &stats[3], &stats[4], &stats[5],\
		                &stats[6], &stats[7], &stats[8]) == 9){
		fclose(stats_file);
	}else{
		stats_file = fopen(file, "a+");
		fclose(stats_file);
		for(int i = 0; i < 9; i++){
			stats[i] = 0;
		}
	}
}
