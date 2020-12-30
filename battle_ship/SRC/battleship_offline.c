/*
 * *slaps code* This bad boy can fit so many battleships
 * 
 * suggest setting font size of terminal to 8
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>

#include "ascii_art.h"
#include "battleship.h"
#include "battleship_online.h"

void sigintHandler(int sig_num)
{
	// catches sneaky ctrl+c's

	signal(SIGINT, sigintHandler);
	puts("\nPlease press ctrl+d to exit...\n");
}


int main(int argc, char** argv)
{
	srand((unsigned)time(NULL));
	setlocale(LC_ALL, "");
	signal(SIGINT, sigintHandler);
	init_game();
}

int init_game(void)
{
	/* main menu
	 * gets user input
	 * starts new game or loads a saved game
	 */

	char choice[17] = {'\0'};
	int bad_input = 0;
	int turn = 0;
	
	system("clear");
	
	while(1){
		print_sick_logo();
		
		if(bad_input){
			puts("Bad input, try again...");
			bad_input = 0;
		}
		puts("Enter your choice_");
		puts("1: New Game?");
		puts("2: Load Game?");
		puts("3: Play Online?");
		printf("> ");

		if(get_input(choice)){
			exit(1);
		}
		
		// check user input
		if(atoi(choice) == 1){
			new_game();
			break;
		}else if(atoi(choice) == 2){
			struct player p_one;
			struct player p_two;
			
			turn = load_game(&p_one, &p_two);
			
			if(turn == 2){
				puts("No save file...");
				sleep(3);
				new_game();
				break;
			}
			game_loop(&p_one, &p_two, turn);
			save_game(&p_one, &p_two, turn);
			free_grids(&p_one, &p_two);
			break;
		}else if(atoi(choice) == 3){
			o_init_game();
		}else{
			system("clear");
			bad_input = 1;
			continue;
		}
	}
	
	return 0;
}

int new_game(void)
{
	/* new game
	 * inits player one and player two
	 */

	int x_size = 10;
	int y_size = 10;
	
	srand((unsigned)time(NULL));
	x_size = (rand() % (16)) + 10;
	y_size = (rand() % (16)) + 10;
	
	struct player p_one;
	struct player p_two;
	
	p_one.boats_health = 19;
	p_two.boats_health = 19;
	
	p_one.guess = generate_grid(x_size, y_size, 0);
	p_one.boats = generate_grid(x_size, y_size, 1);

	p_two.guess = generate_grid(x_size, y_size, 0);
	p_two.boats = generate_grid(x_size, y_size, 1);
	
	if(place_boats(&p_one.boats, 1)){
		free_grids(&p_one, &p_two);
		exit(1);
	}

	if(place_boats(&p_two.boats, 2)){
		free_grids(&p_one, &p_two);
		exit(1);
	}
	
	game_loop(&p_one, &p_two, 0);
	
	free_grids(&p_one, &p_two);
	
	return 0;
}

int game_loop(struct player *p_one, struct player *p_two, int turn)
{
	/* main game loop
	 * gets x/y coodinates from players or calls save_game
	 */

	char input_x[17] = {'\0'};
	char input_y[17] = {'\0'};
	
	int guess_x = 0;
	int guess_y = 0;
	int bad_input = 0;
	int game_over = 0;
	
	struct player *players[2] = {p_one, p_two};
	
	while(!game_over){
		system("clear");

		print_sick_logo();
		print_grid(&players[turn]->guess);

		puts("Enter \"s\" to save and exit game");
		
		if(bad_input){
			puts("Bad guess. Try again...");
			bad_input = 0;
		}
		
		printf("Enter x coordinate guess (1 - %d)\n", p_one->guess.x);
		printf("Player_%d-> ", turn + 1);
		
		// get input
		if(get_input(input_x)){
			free_grids(p_one, p_two);
			exit(1);
		}
		
		// check for s
		if(strchr(input_x, 's')){
			save_game(p_one, p_two, turn);
			free_grids(p_one, p_two);
			exit(1);
		}
		
		// check if guess is int
		if(atoi(input_x) == 0){
			bad_input = 1;
			continue;
		}
		
		// set guess x
		guess_x = atoi(input_x) - 1;
		
		system("clear");
		print_sick_logo();
		print_grid(&players[turn]->guess);
		
		printf("Enter y coordinate guess (1 - %d)\n", p_one->guess.y);
		printf("Player_%d-> ", turn + 1);
		
		// get input
		if(get_input(input_y)){
			free_grids(p_one, p_two);
			exit(1);
		}
		
		// check for s
		if(strchr(input_y, 's')){
			save_game(p_one, p_two, turn);
			free_grids(p_one, p_two);
			exit(1);
		}
		
		// check if guess is int
		if(atoi(input_y) == 0){
			bad_input = 1;
			continue;
		}
		
		// set guess y
		guess_y = atoi(input_y) - 1;
		
		// check if guess is out of bounds
		if(guess_x > p_one->boats.x - 1 || guess_y > p_one->boats.y - 1){
			bad_input = 1;
			continue;
		}
		
		// check if guess is out of bounds
		if(guess_x < 0 || guess_y < 0){
			bad_input = 1;
			continue;
		}
		
		// check if hit or miss
		game_over = check_guess(guess_x, guess_y,\
					players[turn ^ 1 << 0], players[turn]);

		// pause to show player guess response
		system("clear");
		print_sick_logo();
		print_grid(&players[turn]->guess);
		
		puts("Press enter to continue...");
		if(get_input(input_x)){
			free_grids(p_one, p_two);
			exit(1);
		}
		
		// change turn
		turn ^= 1 << 0;
	}
	
	// game over, print obnoxious win text
	winner_winner_chicken_dinner();
	
	return 0;
}

int check_guess(int x_guess, int y_guess, struct player *opponent,\
		struct player *guesser)
{
	/* checks player guess
	 * updates grids based on guess
	 */
	
	if(opponent->boats.grid[y_guess][x_guess] != L' ' && \
	   opponent->boats.grid[y_guess][x_guess] != L'🌀'){
		// player got a hit
		
		// change opponet boats map at coord to ' ' (space)
		opponent->boats.grid[y_guess][x_guess] = ' ';
		
		// decrement opponent boats_health
		opponent->boats_health -= 1;
		
		// prints hit ascii art
		print_hit();
		
		// change player guess map at coord to 'H'	
		if(guesser->guess.grid[y_guess][x_guess] != 'H'){
			guesser->guess.grid[y_guess][x_guess] = 'H';
		}
	}else{
		// player missed
		
		// print miss ascii art
		print_miss();
		
		// change player guess map at coord to 'M'
		if(guesser->guess.grid[y_guess][x_guess] != 'H'){
			guesser->guess.grid[y_guess][x_guess] = 'M';
		}
	}
	
	// return 1 for win condition
	if(opponent->boats_health == 0){
		return 1;
	}
	
	return 0;
}

struct map_grid generate_grid(int x_axis, int y_axis, int is_boats)
{
	/* mallocs 2d array for map grid
	 * populates 2d array with whitespace
	 */
	struct map_grid map;
	
	int temp = 0;
	
	map.x = x_axis;
	map.y = y_axis;
	
	// malloc 'y axis'
	map.grid = malloc(map.y * sizeof(wchar_t *));
	
	// malloc 'x axis'
	for(int i = 0; i < map.y; i ++){
		map.grid[i] = malloc(map.x * sizeof(wchar_t));
	}
	
	// populate grid with whitespace and random obstacles
	for(int i = 0; i < map.y; i++){
		for(int j = 0; j < map.x; j++){
			
			temp = (rand() % (100));
			if(is_boats){
				if(temp < 25){
					map.grid[i][j] = L'🌀';
				}else{
					map.grid[i][j] = ' ';
				}
			}else{
				map.grid[i][j] = ' ';
			}
			
		}
	}
	
	return map;
}

void print_grid(struct map_grid *map)
{
	/* prints map grid with flourishes and numbers
	 */
	
	char temp[5];
	sprintf(temp, "%d", map->y);
	
	// gets # of digits in largest number for margin
	int marg = strlen(temp);  
	
	// print first line
	printf("%s", "\xE2\x94\x8c");
	printf("%s", "\xE2\x94\x80");
	printf("%s", "\xE2\x94\x80");
	
	for(int i = 0; i < map->x * 4 + 1; i++){
		if(i > 0 && i % 4 == 1){
				printf("%s", "\xE2\x94\xac");
			}else{
				printf("%s", "\xE2\x94\x80");
			}
	}
	
	printf("%s", "\xE2\x94\x90");
	
	puts("");
	
	// print second line
	printf("%s   %s","\xE2\x94\x82", "\xE2\x94\x82");
	
	for(int i = 0; i < map->x; i ++){
		printf("%*d %s", marg, i + 1, "\xE2\x94\x82");
	}
	
	puts("");
	
	// print third line
	printf("%s", "\xE2\x94\x9c");
	
	for(int i = 0; i < map->x * 4 + 2; i++){
		if(i > 0 && i % 4 == 3){
				printf("%s", "\xE2\x94\xbc");
			}else{
				printf("%s", "\xE2\x94\x80");
			}
	}
	
	printf("%s", "\xE2\x94\x80");
	printf("%s", "\xE2\x94\xa4");
	
	puts("");
	
	// print the rest of the grid
	for(int i = 0; i < map->y; i++){
		printf("%s%*d %s", "\xE2\x94\x82", marg, i + 1, "\xE2\x94\x82");
		
		for(int j = 0; j < map->x; j++){
			if(map->grid[i][j] == L'⏅'){
				printf(" %s %s", "\xE2\x8F\x85", "\xE2\x94\x82");
			}else if(map->grid[i][j] == L'⛴'){
				printf(" %s %s", "\xE2\x9B\xB4", "\xE2\x94\x82");
			}else if(map->grid[i][j] == L'🛥'){
				printf(" %s %s", "\xF0\x9F\x9B\xA5", "\xE2\x94\x82");
			}else if(map->grid[i][j] == L'🛳'){
				printf(" %s %s", "\xF0\x9F\x9B\xB3", "\xE2\x94\x82");
			}else{
				printf("%*lc %s", marg, map->grid[i][j], "\xE2\x94\x82");
			}
		}
		
		puts("");
		
		for(int j = 0; j < map->x * 4 + 5; j++){
			if(i < map->y - 1 &&  j == 0){
				printf("%s", "\xE2\x94\x9c");
				continue;
			}
			
			if(i == map->y - 1 && j == 0){
				printf("%s", "\xE2\x94\x94");
				continue;
			}
			
			if(j > 0 && j % 4 == 0){
				if(j == (map->x * 4 + 4) && i == map->y - 1){
					printf("%s", "\xE2\x94\x98");
					break;
				}
				
				if(i < map->y - 1 && j == (map->x * 4 + 4)){
					printf("%s", "\xE2\x94\xa4");
				}else{
					if(i == map->y - 1){
						printf("%s", "\xE2\x94\xb4");
					}else{
						printf("%s", "\xE2\x94\xbc");
					}
				}
			}else{
				printf("%s", "\xE2\x94\x80");
			}
		}
		puts("");
	}
}

int update_map(int len, wchar_t boat, int *coords, struct map_grid *map)
{
	/* places boat chars on player map if there is space
	 */

	int x1 = coords[0];
	int y1 = coords[1];
	int x2 = coords[2];
	int y2 = coords[3];
	
	// check for diagonal coordinates
	if(x1 != x2 && y1 != y2){
		return 1;
	}
	
	// reverses coordinates to be low to high
	if((x1 + y1) > (x2 + y2)){
		x1 = coords[2];
		y1 = coords[3];
		x2 = coords[0];
		y2 = coords[1];
	}
	
	// check if there is space for boat
	for(int i = 0; i < len; i++){
		if(x1 == x2){
			if(map->grid[y1 + i][x1] != ' '){
				return 1;
			}
		}else{
			if(map->grid[y1][x1 + i] != ' '){
				return 1;
			}
		}
	}
	
	// place boat
	for(int i = 0; i < len; i++){
		if(x1 == x2){
			map->grid[y1 + i][x1] = boat;
		}else{
			map->grid[y1][x1 + i] = boat;
		}
	}
	
	return 0;
}

int get_coords(int num, wchar_t boat_char, int *coords, struct map_grid *map,\
	       int player_num, const char *boat)
{
	/* gets coordinates from player to place boats on their boat grid
	 */

	char coord[17] = {'\0'};
	
	char axis[4] = {'x', 'y', 'x', 'y'};
	
	int lens[4] = {map->x, map->y, map->x, map->y};
	
	static int bad_input = 0;
	
	int result = 0;
	
	for(int i = 0; i < 4; i++){
		system("clear");
		print_sick_logo();
		print_grid(map);
		
		if(bad_input){
			puts("Bad input, try again...");
			printf("You need to input the coordinates at the ");
			printf("ends of %d spaces\n\n", num);
			bad_input = 0;
		}
		
		printf("Enter %s coordinates.\n", boat);
		printf("The %s is %d spaces long.\n", boat, num);
		
		if(i < 2){
			printf("Enter %c coordinate # 1 (1 - %d)\n", axis[i],\
			       lens[i]);
		}else{
			printf("Enter %c coordinate # 2 (1 - %d)\n", axis[i],\
			       lens[i]);
		}
		
		printf("Player_%d-> ", player_num);
		
		if(get_input(coord)){
			return 2;
		}
		
		if(atoi(coord) == 0){
			bad_input = 1;
			return 1;
		}
		
		coords[i] = atoi(coord);
		
		// check if coordinates are out of bounds
		if(i % 2 == 0){
			if(coords[i] < 1 || coords[i] > map->x){
				bad_input = 1;
				return 1;
			}
		}else{
			if(coords[i] < 1 || coords[i] > map->y){
				bad_input = 1;
				return 1;
			}
		}
		
		// decrement coords to match index
		coords[i]--;
	}
	
	// check if coordinates equals the correct number of spaces
	if(abs((coords[0] + coords[1]) - (coords[2] + coords[3])) + 1 != num){
		bad_input = 1;
		return 1;
	}else{
		result = update_map(num, boat_char, coords, map);
		
		if(result == 1){
			bad_input = 1;
			return 1;
		}else{
			return result;
		}
	}
}

int place_boats(struct map_grid *map, int player_num)
{
	/* loops through boats and calls get_coords
	 */

	char input[17] = {'\0'};
	int boat_lens[] = {2, 2, 3, 3, 4, 5};
	wchar_t boat_chars[] = {L'⏅', L'⛵', L'⛴', L'🛥', L'🛳', L'🚢'};
	const char *boats[] = {"Patrol Boat", "Sumbarine", "Cruiser",\
			       "Destroyer", "Battleship", "Aircraft Carrier"};
			 
	int coords[4] = {0};
	int check = 3;
	
	for(int i = 0; i < 6; i++){
		while(1){
			check = get_coords(boat_lens[i], boat_chars[i],\
					   coords, map, player_num, boats[i]);
			
			if(check == 2){
				return 1;
			}else if(check == 0){
				break;
			}
		}
		system("clear");
		print_sick_logo();
		print_grid(map);
		
		puts("Press enter to continue...");
		
		if(get_input(input)){
			return 1;
		}
	}
	
	return 0;
}

int get_input(char *input)
{
	/* get user input and drain stdin
	 */

	char dumpster[10] = {'\0'};
	
	if(fgets(input, 17, stdin) == NULL){
		return 1;
	}
	
	if(!strchr(input, '\n')){
		while(!strchr(dumpster, '\n')){
			if(fgets(dumpster, 10, stdin) == NULL){
				return 1;
			}
		}
	}
	
	return 0;
}

int free_grids(struct player *one, struct player *two)
{
	/* frees 2d grid arrays
	 */
	struct map_grid maps[4] = {one->guess, one->boats, two->guess,\
				   two->boats};
	
	for(int i = 0; i < 4; i ++){
		for(int j = 0; j < maps[i].y; j++){
			free(maps[i].grid[j]);
		}
	
		free(maps[i].grid);
	}
	
	return 0;
}

int save_game(struct player *p_one, struct player *p_two, int turn)
{
	/* saves current game
	 */

	char file[80] = {'\0'};
	
	strcat(file, getenv("HOME"));
	strcat(file, "/.save");
	
	FILE *fp;
	
	fp = fopen(file, "w");
	
	if(!fp){
		return 1;
	}
	
	struct map_grid maps[4] = {p_one->guess, p_one->boats, p_two->guess,\
				   p_two->boats};
	
	// write x axis length, y axis length, player boat healths and turn
	fprintf(fp, "%d %d %d %d %d\n", p_one->boats.x, p_one->boats.y,\
		p_one->boats_health, p_two->boats_health, turn);
	
	// write player maps
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < p_one->guess.y; j++){
			for(int k = 0; k < p_one->guess.x; k++){
				fprintf(fp, "%lc", maps[i].grid[j][k]);
			}
			fputc(L'\n', fp);
		}
	}

	fclose(fp);
	
	return 0;
}

int load_game(struct player *p_one, struct player *p_two)
{
	/* loads previous save game
	 */

	int temp_x = 0;
	int temp_y = 0;
	int temp_health1 = 0;
	int temp_health2 = 0;
	int temp_turn = 0;
	
	wchar_t *temp_field = NULL;
	
	char file[80] = {'\0'};
	
	strcat(file, getenv("HOME"));
	strcat(file, "/.save");
	
	FILE *fp;
	
	fp = fopen(file, "r");
	
	if(!fp){
		return 2;
	}
	
	// read first line
	if((fscanf(fp, "%d %d %d %d %d", &temp_x, &temp_y, &temp_health1,\
	          &temp_health2, &temp_turn)) != 5){

		fclose(fp);
		return 2;
	}
	
	// checks if saved game is already over
	if(temp_health1 == 0 || temp_health2 == 0){
		fclose(fp);
		return 2;
	}
	
	temp_field = malloc(sizeof(wchar_t) * temp_x + 1);
	
	// move to beginning of next line
	fseek(fp, 1, SEEK_CUR);
	
	// alloc grids
	p_one->guess = generate_grid(temp_x, temp_y, 0);
	p_one->boats = generate_grid(temp_x, temp_y, 0);

	p_two->guess = generate_grid(temp_x, temp_y, 0);
	p_two->boats = generate_grid(temp_x, temp_y, 0);
	
	// set boat healths
	p_one->boats_health = temp_health1;
	p_two->boats_health = temp_health2;
	
	struct map_grid maps[4] = {p_one->guess, p_one->boats, p_two->guess,\
				   p_two->boats};
	
	// read and populate player grids
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < temp_y; j++){
			for(int k = 0; k < temp_x; k++){
				fscanf(fp, "%lc", &temp_field[k]);
			}
		
			fseek(fp, 1, SEEK_CUR);
			
			for(int k = 0; k < temp_x; k++){
				maps[i].grid[j][k] = temp_field[k];
			}
		}
	}
	
	fclose(fp);
	free(temp_field);
	return temp_turn;
}


void print_sick_logo(void)
{
	/* prints the battleship asci art
	 */

	for(int i = 0; i < 67; i++){
		printf("%s", "\xE2\x94\x8b");
	}
	
	puts("");
	
	printf("%s\n", logo_one);
	printf("%s\n", logo_two);
	printf("%s\n", logo_three);
	printf("%s\n", logo_four);
	printf("%s\n", logo_five);
	printf("%s\n", logo_six);
	
	puts("");
	
	for(int i = 0; i < 67; i++){
		printf("%s", "\xE2\x94\x8b");
	}
	puts("");
}


void print_hit(void)
{
	// print hit ascii art

	system("clear");
	
	puts(hit_one);
	puts(hit_two);
	puts(hit_three);
	puts(hit_four);
	puts(hit_five);
	puts(hit_six);
	puts(hit_seven);
	puts(hit_eight);
	puts(hit_nine);
	
	sleep(2);
}

void print_miss(void)
{
	// print miss ascii art

	system("clear");
	
	puts(miss_one);
	puts(miss_two);
	puts(miss_three);
	puts(miss_four);
	puts(miss_five);
	puts(miss_six);
	puts(miss_seven);
	puts(miss_eight);
	puts(miss_nine);
	
	sleep(2);
}

void winner_winner_chicken_dinner(void)
{
	// print obnoxious win text

	system("clear");

	for(int i = 0; i < 1000; i++){
		printf("_-WINNER_-");
		fflush(stdout);
		usleep(20000);
	}
}
