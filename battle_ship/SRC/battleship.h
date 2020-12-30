#pragma once

#include <wchar.h>

// map grid
struct map_grid{
	int x;
	int y;
	wchar_t **grid;
};

// player
struct player{
	struct map_grid guess;
	struct map_grid boats;
	int boats_health;
};

int init_game(void); // 1

int new_game(void); // 2

struct map_grid generate_grid(int, int, int); // 3

int place_boats(struct map_grid *, int); // 4

int get_coords(int, wchar_t, int *, struct map_grid *, int, const char *); // 5

int update_map(int, wchar_t, int *, struct map_grid *); // 6

int game_loop(struct player *, struct player *, int); // 7

int check_guess(int, int, struct player *, struct player *);

void print_grid(struct map_grid *);

int get_input(char *);

int free_grids(struct player *, struct player *);

void print_sick_logo(void);

void print_hit(void);

void print_miss(void);

void winner_winner_chicken_dinner(void);

int save_game(struct player *, struct player *, int);

int load_game(struct player *, struct player *);
