#pragma once

int connect_server(int *, char *, int *);

int o_init_game(void);

int o_new_game(int, int, int, int);

int o_load_game(int, int);

int o_save_game(int, struct player *);

int o_game_loop(struct player *, int, int, int);

int o_check_guess(int, int, struct player *, int);

int free_grid(struct player *);

void clear_buffer(char *, int);

int int_from_str(char *, int *);

void o_print_hit(void);

void o_print_miss(void);

void loser_loser_you_suck(void);
