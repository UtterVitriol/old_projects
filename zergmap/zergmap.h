
#include <stdint.h>
#include <stdbool.h>

typedef struct Zerg {
	double lat;
	double lon;
	float alt;
	float acc;
	int health;
	int max_health;
	bool has_gps;
	bool deleted;
	uint16_t id;
} Zerg;

typedef struct Zergs {
	Zerg **zergs;
	int count;
	int max;
	int start_size;
	int del_count;
} Zergs;

typedef struct Graph Graph;

Zergs *init_zergs(int size);
Zerg *create_zerg(uint16_t id);
bool add_zerg(Zergs *zergs, Zerg *zerg, bool is_gps);
void handle_args(int argc, char **argv, double *health, int *del_limit);

void cleanup_zergs(Zergs *zergs);
void print_dead(Zergs *zergs);
void print_health(Zergs *zergs, double health);
void print_no_gps(Zergs *zergs);

double get_distance(Zerg *zerg1, Zerg *zerg2);
void print_dists(Zergs *zergs);

void free_zergs(Zergs *zergs);

Graph *get_nodes_to_kill(Graph *graph, Zergs *zergs);
void print_help(void);