//https://stackoverflow.com/questions/2597608/c-socket-connection-timeout
//https://www.geeksforgeeks.org/socket-programming-cc/

/*
 * *slaps code* This bad boy probably shouldn't work at all....
 * 
 * suggest setting font size of terminal to 8
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <fcntl.h>
#include <wctype.h>

#include "ascii_art.h"
#include "battleship.h"
#include "battleship_online.h"

#define PORT 6666

int connect_server(int *sock, char *ip, int *player)
{
	// connect to server

	struct sockaddr_in serv_addr;
	char buffer[100] = {0};
	fd_set fdset;
	struct timeval tv;
	
	// create socket
	if((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		puts("Socket Error...");
		return 2;
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// check address
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		puts("Invalid Address...");
		return 2;
	}
	
	// set socket to non-blocking
	long arg;
	arg = fcntl(*sock, F_GETFL, NULL);
	arg |= O_NONBLOCK;
	fcntl(*sock, F_SETFL, arg);
	
	printf("Connecting to server...\n");
	
	// try to connect...timeout after 3 seconds
	int res;
	res = connect(*sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(res < 0){
		FD_ZERO(&fdset);
		FD_SET(*sock, &fdset);
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		res = select(*sock + 1, NULL, &fdset, NULL, &tv);
		if(res > 0)
		{
			int so_error;
			socklen_t len = sizeof(so_error);
			
			if(getsockopt(*sock, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0){
				puts("Error in  getsockopt");
				return 2;
			}
			
			if(so_error){
				printf("\nError in connection...\n");
				close(*sock);
				return 2;
			}
		}else if(res < 0){
			puts("Error in connection..\n");
			return 2;
		}else{
			printf("Connection timed out...");
			return 2;
		}
	}
	
	// set socket back to blocking
	arg = fcntl(*sock, F_GETFL, NULL);
	arg &= (~O_NONBLOCK);
	fcntl(*sock, F_SETFL, arg);
	
	puts("Connected to server...");
	puts("Waiting for other player...");
	
	read(*sock, buffer, 100);
	
	*player = atoi(buffer);
	
	
	return 0;
}

int o_init_game(void)
{
	/* get ip of server
	 */

	char choice[17] = {'\0'};
	char buffer[10] = {'\0'}, *b_ptr = NULL;
	int coords[2] = {0};
	int sock, player;
	
	system("clear");
	print_sick_logo();
	
	while(1){
		puts("");
		puts("Enter ip address of server");
		printf("> ");
		
		if(get_input(choice)){
			exit(1);
		}
		
		choice[strlen(choice) - 1] = '\0';

		if(connect_server(&sock, choice, &player) != 0){
			puts("");
			continue;
		}
		
		clear_buffer(buffer, 10);
		read(sock, buffer, 10);
		
		if(atoi(buffer) == 1){
			system("clear");
			print_sick_logo();
			
			read(sock, buffer, 1024);
			
			coords[0] = strtol(buffer, &b_ptr, 10);
			int_from_str(b_ptr, &coords[1]);
			
			puts("Sarting Game....");
			
			o_new_game(player, coords[0], coords[1], sock);
			
			break;
		}else if(atoi(buffer) == 2){
			puts("Receiving saved game..");
			o_load_game(sock, player);
			
			break;
		}else{
			puts("Something broke...");
			break;
		}
	}
	return 0;
}

int o_new_game(int player, int x, int y, int sock)
{
	/* new game
	 * inits player one and player two
	 */

	struct player p_one;
	char buffer[3] = {0};
	
	int p = 1;
	
	if(player == 1){
		p = 2;
	}
	
	p_one.boats_health = 19;
	
	p_one.guess = generate_grid(x, y, 0);
	p_one.boats = generate_grid(x, y, 1);
	
	if(place_boats(&p_one.boats, player)){
		send(sock, "1", 1, 0);
		close(sock);
		free_grid(&p_one);
		exit(1);
	}
	
	send(sock, "0", 1, 0);
	
	system("clear");
	print_sick_logo();
	printf("\nWaiting for other player...\n");
	
	read(sock, buffer, 3);
	
	if(atoi(buffer)){
		printf("Player %d disconnected...", p);
		close(sock);
		free_grid(&p_one);
		exit(1);
	}
	
	read(sock, buffer, 3);

	if(player == 1){
		o_game_loop(&p_one, player, 0, sock);
	}else{
		o_game_loop(&p_one, player, 1, sock);
	}

	return 0;
}

int o_game_loop(struct player *p_one, int player, int turn, int sock)
{
	/* main game loop
	 * gets x/y coodinates from players or calls o_save_game
	 */

	char input_x[17] = {'\0'};
	char input_y[17] = {'\0'};
	char guess[17] = {'\0'};
	char buffer[17] = {'\0'}, *b_ptr;
	
	int guess_x = 0;
	int guess_y = 0;
	int bad_input = 0;
	int game_over = 0;
	int p = 2;
	
	int sockstate = 0;
	int error = 0;
	socklen_t len = sizeof(error);
	
	
	if(player == 2){
		p = 1;
	}
	
	while(!game_over){
		
		
		if(turn == 1){
			system("clear");
			print_sick_logo();
			print_grid(&p_one->guess);
			printf("Waiting for player %d...\n", p);
			read(sock, buffer, 10);
			
			if(atoi(buffer) == 100){
				o_save_game(sock, p_one);
				free_grid(p_one);
				close(sock);
				printf("Player %d disconnected...\n", p);
				exit(1);
			}
			
			guess_x = strtol(buffer, &b_ptr, 10);
			int_from_str(b_ptr, &guess_y);
			
			clear_buffer(buffer, 17);
			
			// check that shit
			if(o_check_guess(guess_x, guess_y, p_one, sock)){
				free_grid(p_one);
				close(sock);
				loser_loser_you_suck();
				exit(1);
			}
			
			// pause to show player guess response
			system("clear");
			print_sick_logo();
			print_grid(&p_one->boats);
			
			puts("Press enter to continue...");
			if(get_input(input_x)){
				send(sock, "3", 1, 0);
				o_save_game(sock, p_one);
				close(sock);
				free_grid(p_one);
				exit(1);
			}
			
			turn = 0;
			
		}
		
		system("clear");
		print_sick_logo();
		print_grid(&p_one->guess);
		
		if(bad_input){
			puts("Bad guess. Try again...");
			bad_input = 0;
		}
		
		printf("Enter x coordinate guess (1 - %d)\n", p_one->guess.x);
		printf("Player_%d-> ", player);
		
		// get input
		if(get_input(input_x)){
			send(sock, "3", 1, 0);
			o_save_game(sock, p_one);
			close(sock);
			free_grid(p_one);
			exit(1);
		}
		
		// check if guess is int
		if(atoi(input_x) == 0){
			bad_input = 1;
			continue;
		}
		
		// set guess x
		guess_x = atoi(input_x) - 1;
		
		clear_buffer(input_x, 17);
		
		system("clear");
		print_sick_logo();
		print_grid(&p_one->guess);
		
		printf("Enter y coordinate guess (1 - %d)\n", p_one->guess.y);
		printf("Player_%d-> ", player);
		
		// get input
		if(get_input(input_y)){
			send(sock, "3", 1, 0);
			o_save_game(sock, p_one);
			close(sock);
			free_grid(p_one);
			exit(1);
		}
		
		// check if guess is int
		if(atoi(input_y) == 0){
			bad_input = 1;
			continue;
		}
		
		// set guess y
		guess_y = atoi(input_y) - 1;
		
		clear_buffer(input_y, 17);
		
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
		sprintf(guess, "%d %d", guess_x, guess_y);
		
		send(sock, guess, 6, 0);
		
		clear_buffer(guess, 17);

		while(read(sock, buffer, 6) < 0){}
		
		if(atoi(buffer) == 1){
			p_one->guess.grid[guess_y][guess_x] = 'H';
			system("clear");
			o_print_hit();
			sleep(2);
		}else if(atoi(buffer) == 2){
			p_one->guess.grid[guess_y][guess_x] = 'M';
			system("clear");
			o_print_miss();
			sleep(2);
		}else if(atoi(buffer) == 3){
			close(sock);
			free_grid(p_one);
			winner_winner_chicken_dinner();
			exit(1);
		}else if(atoi(buffer) == 100){
			close(sock);
			free_grid(p_one);
			printf("Player %d disconnected...\n", p);
			exit(1);
		}
		
		clear_buffer(buffer, 17);
		
		turn = 1;
	}
	
	// game over, print obnoxious win text
	winner_winner_chicken_dinner();
	
	return 0;
}

int o_check_guess(int x_guess, int y_guess, struct player *p_one, int sock)
{
	/* checks opponent guess
	 * updates grids based on guess
	 */

	if(p_one->boats.grid[y_guess][x_guess] != L' ' && \
	   p_one->boats.grid[y_guess][x_guess] != L'🌀' && \
	   p_one->boats.grid[y_guess][x_guess] != L'💥'){
		// opponent got a hit
		
		// change player boats map at coord to '💥' (space)
		p_one->boats.grid[y_guess][x_guess] = L'💥';
		
		// decrement player boats_health
		p_one->boats_health -= 1;
		
		// send 1 for win condition
		if(p_one->boats_health == 0){
			send(sock, "3", 1, 0);
			return 1;
		}
		
		//send hit
		send(sock, "1", 1, 0);
		
		// prints hit ascii art
		o_print_hit();
		sleep(2);
	}else{
		// player missed
		
		p_one->boats.grid[y_guess][x_guess] = L'💥';
		//send miss
		send(sock, "2", 1, 0);
		
		// print miss ascii art
		o_print_miss();
		sleep(2);
	}

	return 0;
}

int o_load_game(int sock, int player)
{
	// loads previous save game

	int temp_x = 0;
	int temp_y = 0;
	int temp_health = 0;
	int temp_turn = 0;
	
	struct player p_one;
	
	char *temp_field = NULL;
	
	wchar_t *temp_wfield = NULL;
	
	char buffer[10] = {'\0'}, *b_ptr = NULL;
	
	// get coords
	read(sock, buffer, 10);

	temp_x = strtol(buffer, &b_ptr, 10);
	int_from_str(b_ptr, &temp_y);
	
	int size = ((((temp_x + 1) * temp_y) * 2) - 1) * 4 ;
	int wsize = (((temp_x + 1) * temp_y) * 2) - 1;
	
	if((temp_field = calloc(sizeof(char), size + 1)) == NULL){
		puts("bad calloc");
	}
	
	if((temp_wfield = calloc(sizeof(wchar_t), wsize + 2)) == NULL){
		puts("bad calloc 2");
	}
	
	
	// alloc grids
	p_one.guess = generate_grid(temp_x, temp_y, 0);
	p_one.boats = generate_grid(temp_x, temp_y, 0);
	
	// get health
	clear_buffer(buffer, 10);
	read(sock, buffer, 10);
	temp_health = strtol(buffer, &b_ptr, 10);
	
	// set boat healths
	p_one.boats_health = temp_health;
	
	// get turn
	clear_buffer(buffer, 10);
	read(sock, buffer, 10);
	temp_turn = strtol(buffer, &b_ptr, 10);
	
	// get maps
	read(sock, temp_field, size);
	swprintf(temp_wfield, wsize + 1, L"%s", temp_field);
	
	struct map_grid *temp_map;
	temp_map = &p_one.guess;
	int y = 0;
	int x = 0;
	
	// populate grids
	for(int i = 0; i < wsize; i++){
		
		// check if end of first grid
		if(y == temp_y){
			temp_map = &p_one.boats;
			y = 0;
			x = 0;
		}
		
		// new line
		if(temp_wfield[i] == L'\n'){
			x = 0;
			y++;
			continue;
		}
		
		// debug code
		if(x == temp_x || y == temp_y + 1){
			puts("your logic is trash");
			printf("x: %d y: %d", x, y);
			break;
		}
		
		temp_map->grid[y][x] = temp_wfield[i];
		x++;
	}

	free(temp_field);
	free(temp_wfield);
	o_game_loop(&p_one, player, temp_turn, sock);
	free_grid(&p_one);
	return 0;
}

int o_save_game(int sock, struct player *p_one)
{
	// sends current game state to server

	char *grids = NULL;
	wchar_t *wgrids = NULL;
	int temp_x = p_one->guess.x;
	int temp_y = p_one->guess.y;
	
	int size = (((((temp_x + 1) * temp_y)) * 2) ) * 4 ;
	int wsize = ((((temp_x + 1) * temp_y)) * 2);
	
	if((grids = calloc(sizeof(char), size + 1)) == NULL){
		puts("bad calloc");
	}
	
	if((wgrids = calloc(sizeof(wchar_t), wsize + 1)) == NULL){
		puts("bad calloc 2");
	}
	
	int count = 0;
	
	for(int i = 0; i < temp_y; i++){
		for(int j = 0; j < temp_x; j++){
			wgrids[count] = p_one->guess.grid[i][j];
			count++;
		}
		wgrids[count] = L'\n';
		count++;
	}
	
	for(int i = 0; i < temp_y; i++){
		for(int j = 0; j < temp_x; j++){
			wgrids[count] = p_one->boats.grid[i][j];
			count++;
		}
		
		wgrids[count] = L'\n';
		count++;
	}
	
	sprintf(grids, "%ls", wgrids);
	
	send(sock, grids, strlen(grids), 0);
	
	free(grids);
	free(wgrids);
	return 0;
}

int free_grid(struct player *one)
{
	/* frees 2d grid arrays
	 */
	struct map_grid maps[2] = {one->guess, one->boats};
	
	for(int i = 0; i < 2; i ++){
		for(int j = 0; j < maps[i].y; j++){
			free(maps[i].grid[j]);
		}
	
		free(maps[i].grid);
	}
	
	return 0;
}

void o_print_hit(void)
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
	
}

void o_print_miss(void)
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
	
}

void loser_loser_you_suck(void)
{
	// print obnoxious win text

	system("clear");

	for(int i = 0; i < 1000; i++){
		printf("_-LOSER_-");
		fflush(stdout);
		usleep(20000);
	}
}

void clear_buffer(char *buffer, int len)
{
	// sets buffer to \0's

	for(int i = 0; i < len; i++){
		buffer[i] = '\0';
	}
}

int int_from_str(char *line, int *number)
{
	// gets next int from string

	char *position = NULL;
	position = line;	
	while(*position){
		if(isdigit(*position)){
			*number = strtol(position, &position, 10);
			return 0;
		}else{
			position++;
		}
	}
	return 1;
}
