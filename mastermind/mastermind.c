/* mastermind game
 * 
 * Run to play manually or run with -autoplay option --
 * to have the game play itself.
 * 
 * Create .mm file with secret code and the game will read it --
 * instead of generating a random secret code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

void gen_code(char *code);

int get_input(char *guess);

int get_bot_input(char *bot_guess, char *guess);

int bot_eval(int *red, int *white, char *bot_guess, int *nums);

int check_guess(char *code, char *guess, int *red, int *white);

int game_loop(char *code, int *count, int *guess_time,\
	      char *bot_guess, int *nums, int *auto_play);

int main(int argc, char **argv)
{
	/* checks args, generates 0-10000 int array, calls gen_code and 
	 * game_loop
	 */

	int auto_play = 0, error = 0;
	
	
	if(argc > 1){
		if(!strcmp(argv[1], "-autoplay")){
			auto_play = 1;
		}else{
			error = 1;
		}
	}

	if(argc > 2 || error){
		puts("Usage: ./mastermind [-autoplay]");
		return 0;
	}

	// array to test bot guess
	int nums[10000], *pnums;
	pnums = nums;
	for(int i = 0; i < 10000; i ++){
		*pnums++ = i;
	}

	int guess_time = 0, count = 0;
	char code[5] = {'\0'}, bot_guess[6] = {'0', '0', '1', '1'};
	gen_code(code);

	while(1){
		if(game_loop(code, &count, &guess_time,\
		   bot_guess, nums, &auto_play) == 0)
			break;
	}
}


void gen_code(char *code)
{
	/* checks for valid entry in .mm file, quits if bad input
	 * generates random code if no .mm file
	 */
	 
	FILE *fp;

	fp = fopen(".mm", "r");
	int error = 0;
	if(fp){
		char c;
		int count = 0;
		for(int i = 0; i < 5; i ++){
			c = fgetc(fp);
			if(i == 4 && c != '\n'){
				error = 1;
			}
			if(i < 4){
				if(c < '0' || c > '9'){
					error = 1;
				}else{
					code[i] = c;
				}
			}
			if(error){
				puts("Invalid entry in .mm");
				exit(1);
			}
			count++;
		}
		fclose(fp);
		return;
	}

	srand((unsigned)time(NULL)); 

	// generates random ints for secret code
	for(int i = 0; i < 4; i ++){
		code[i] = ((rand() % 10)) + '0';
	}
}

int get_bot_input(char *bot_guess, char *guess)
{
	// copies bot_guess into guess

	strcpy(guess, bot_guess);
	printf("Guess a number: %s\n", guess);
	return 0;
}

int bot_eval(int *red, int *white, char *bot_guess, int *nums)
{
	/* determines next guess
	 * based on Donald Knuth's Five-guess algorithm
	 */

	int temp_r, temp_w, elims = 0, elims1 = 0;
	temp_r = *red;
	temp_w = *white;
	char temp_guess[6], temp_guess1[6];
	
	// remove numbers that don't get same r/w result as last guess
	for(int i = 0; i < 10000; i++){
		if(nums[i] < 10000){
			char temp1[6];
			sprintf(temp1, "%04d", nums[i]);
			check_guess(temp1, bot_guess, red, white);
			if(*red != temp_r || *white != temp_w){
				nums[i] = 10000;
			}
		}
	}

	/* pick next guess by finding guess that will eliminate the most
	   other guesses
	 */
	for(int i = 0; i < 10000; i++){
		for(int j = 9999; j >=0; j--){
			if(nums[j] != 0 && nums[j] < 10000){
				if(nums[i] < 10000 && i < 10000){
					char temp3[6];
					sprintf(temp3, "%04d", nums[i]);
					char temp2[6];
					sprintf(temp2, "%04d", nums[j]);
					check_guess(temp2, temp3, red, white);
					strcpy(temp_guess1, temp3);
					if(*red != temp_r || *white != temp_w)
						elims1 += (*red + *white);
				}
			}
		}
		if(elims1 > elims){
			strcpy(temp_guess, temp_guess1);
			elims = elims1;
		}
		elims1 = 0;
	}
	
	
	strcpy(bot_guess, temp_guess);  // copy guess to bot_guess

	return 0;
}


int get_input(char *guess)
{
	// gets user input, validates it

	char drain[100] = {'\0'};
	printf("Guess a number: ");
	fgets(guess, 6, stdin);
	if(!strchr(guess, '\n')){
		while(!strchr(drain, '\n')){
			fgets(drain, sizeof(drain), stdin);
		}
	}

	for(int i = 0; i < 4; i++){
		if(guess[i] < '0' || guess[i] > '9'){
			puts("Please enter four digits...");
			return 1;
		}
	}
	if(guess[strlen(guess) - 1] != '\n'){
		puts("Too many digits...");
		return 1;
	}

	return 0;
}

int check_guess(char *code, char *guess, int *red, int *white)
{
	// checks guess for matching numbers

	*red = 0;
	*white = 0;
	char  guess_index[5] = {'\0'}, *pguess_index,\
	      code_index[5] = {'\0'}, *pcode_index;
	pguess_index = guess_index;
	pcode_index = code_index;

	// checks for reds and saves indexs that don't match
	for(int i = 0; i < 4; i++){
		if(guess[i] == code[i])
			*red += 1;
		else{
			*pguess_index++ = guess[i];
			*pcode_index++ = code[i];
		}
	}

	// Checks for whites and consumes code at index if match
	for(int i = 0; i < (int)strlen(guess_index); i++){
		for(int j = 0; j < (int)strlen(code_index); j++){
			if(guess_index[i] == code_index[j]){
				*white += 1;
				code_index[j] = 'o';
				break;
			}
		}
	}

	return 0;
}

int game_loop(char *code, int *count, int *guess_time,\
	      char *bot_guess, int *nums, int *auto_play)
{
	/* main game loop - handles guess time and checks win condition
	 * calls get_input, check_guess, get_bot_input, bot_eval
	 */

	time_t tme1, tme2;
	int red = 0, white = 0;
	char guess[6] = {'\0'};

	time(&tme1);

	if(*auto_play == 0){
		while(1){
			if(get_input(guess) == 0)
				break;
		}
	}else{
		get_bot_input(bot_guess, guess);
	}

	*count += 1;

	check_guess(code, guess, &red, &white);

	if(red != 4){
		if(red == 0 && white == 0){
			puts("No matches");
			if(*auto_play == 1)
				bot_eval(&red, &white, bot_guess, nums);
			time(&tme2);
			*guess_time += (int)tme2 - (int)tme1;
		}else{
			printf("red: %d, white: %d\n", red, white );
			if(*auto_play == 1)
				bot_eval(&red, &white, bot_guess, nums);
			time(&tme2);
			*guess_time += (int)tme2 - (int)tme1;
		}
		return 1;
	}

	if(red == 4){
		time(&tme2);
		*guess_time += (int)tme2 - (int)tme1;
		printf("red: %d\n", red);
		if(*count == 1)
			printf("You win! It took you %d guess.\n", *count);
		else{
			printf("You win! ");
			printf("It took you %d guesses.\n", *count);
		}
		double temp_guess_time = *guess_time;
		double temp_count = *count;
		double average = temp_guess_time / temp_count;
		printf("Average time to guess: %.2f seconds.\n", average);
		return 0;
	}

	return 1;
}

