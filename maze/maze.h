#pragma once

typedef struct Map {
	char **grid;
	int x;
	int y;
	int start_x;
	int start_y;
	int finish_x;
	int finish_y;
	bool has_start;
	bool has_finish;
} Map;

int handle_args(int argc, char **argv, bool *flags);
void print_help(void);

Map *read_maze(char *file_name, bool *flags);
Map *init_map(FILE *fp);
void get_map_size(FILE *fp, Map *map);
void free_map(Map *map);
bool validate_line(char *line, int line_num, Map *map, bool *flags);
bool update_map(Map *map, Graph *graph, bool *flags);
void print_map(Map *map);

bool is_in_bounds(int src, int dst, Map *map);
int pos_from_xy(Map *map, int x, int y);

bool check_index(Graph *graph, Map *map, bool *flags, int x_src, int y_src,
		 int x_dst, int y_dst);

Graph *gen_graph_basic(Map *map);
Graph *gen_graph_advanced(Map *map, bool *flags);