/*
 *usage: ./maze [-wdD] [map filename]
 *w - maps can have '~' water, takes 3x as long to cross
 *
 *d - maps can have open '/' and closed '+' doors
 *    closed doors take an extra step to cross
 *
 *D - paths can go through walls, takes 11x as long to cross
 *    -D will result in MORE map restrictions...
 *    If a wall can be dug through that leads to
 *    A space that has a hole to the edge of the map,
 *    it could be considered an invalid map.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "graph.h"
#include "maze.h"

int main(int argc, char **argv)
{
	bool flags[3] = {0};

	// get flags
	if (handle_args(argc, argv, flags)) {
		return 1;
	}

	// generate map
	Map *map = read_maze(argv[optind], flags);

	if (!map) {
		return 1;
	}

	Graph *graph = NULL;

	// generate graph based on flags
	if (flags[0] || flags[1] || flags[2]) {
		graph = gen_graph_advanced(map, flags);
	} else {
		graph = gen_graph_basic(map);
	}

	// edge error checking
	if (!graph) {
		fprintf(stderr, "Invalid map...\n");
		fprintf(stderr, "Map has hole...\n");
		free_map(map);
		return 1;
	}

	// write path to map
	if (!update_map(map, graph, flags)) {
		fprintf(stderr, "Invalid map...\n");
		fprintf(stderr, "Map has hole...\n");
		destroy_graph(graph);
		free_map(map);
		return 1;
	}

	// print map
	print_map(map);

	destroy_graph(graph);

	free_map(map);
}

int handle_args(int argc, char **argv, bool *flags)
{
	/*
	 * gets flags
	 */

	int opt;

	while ((opt = getopt(argc, argv, "wdD")) != -1) {

		switch (opt) {
		case 'w':
			flags[0] = true;
			break;
		case 'd':
			flags[1] = true;
			break;
		case 'D':
			flags[2] = true;
			break;
		case '?':
			fprintf(stderr, "Invalid flag...\n");
			print_help();
			return 1;
		}
	}

	// print help if a single argument isn't leftover
	if ((argc - optind) != 1) {
		print_help();
		return 1;
	}

	return 0;
}

int pos_from_xy(Map *map, int x, int y)
{
	/*
	 * get index for adjacency list from x/y coords
	 */

	return (y * map->x) + x;
}

bool is_in_bounds(int src, int dst, Map *map)
{
	/*
	 * checks if neighbor is in bounds or not
	 * well, tries to...
	 */

	int x_bound = map->x;
	int y_bound = map->y;

	int x_src = src % x_bound;
	int y_src = src / x_bound;

	int x_dst = dst % x_bound;
	int y_dst = dst / x_bound;

	if ((x_dst <= 0 || x_dst >= x_bound - 1) &&
	    map->grid[y_src][x_src] != '#' && map->grid[y_dst][x_dst] != '#') {
		return false;
	}

	if ((y_dst <= 0 || y_dst >= y_bound - 1) &&
	    map->grid[y_src][x_src] != '#' && map->grid[y_dst][x_dst] != '#') {
		return false;
	}

	return true;
}

Graph *gen_graph_basic(Map *map)
{
	/*
	 * generates adjacency list when no flags are present
	 */

	Graph *graph = init_graph((map->x * map->y));

	for (int y = 0; y < map->y; y++) {
		for (int x = 0; x < map->x; x++) {
			if (map->grid[y][x] == '#' || map->grid[y][x] == '\0') {
				continue;
			}

			if ((x + 1) < map->x) {
				if (map->grid[y][x + 1] == '\0') {
					destroy_graph(graph);
					return NULL;
				}
				if (map->grid[y][x + 1] != '#') {
					add_edge(
					    graph, (pos_from_xy(map, x, y)),
					    (pos_from_xy(map, x + 1, y)), 0);
				}
			}

			if ((x - 1) >= 0) {
				if (map->grid[y][x - 1] == '\0') {
					destroy_graph(graph);
					return NULL;
				}
				if (map->grid[y][x - 1] != '#') {
					add_edge(
					    graph, (pos_from_xy(map, x, y)),
					    (pos_from_xy(map, x - 1, y)), 0);
				}
			}

			if ((y + 1) < map->y) {
				if (map->grid[y + 1][x] == '\0') {
					destroy_graph(graph);
					return NULL;
				}
				if (map->grid[y + 1][x] != '#') {
					add_edge(graph, pos_from_xy(map, x, y),
						 pos_from_xy(map, x, y + 1), 0);
				}
			}

			if ((y - 1) >= 0) {
				if (map->grid[y - 1][x] == '\0') {
					destroy_graph(graph);
					return NULL;
				}
				if (map->grid[y - 1][x] != '#') {
					add_edge(graph, pos_from_xy(map, x, y),
						 pos_from_xy(map, x, y - 1), 0);
				}
			}
		}
	}

	return graph;
}

bool check_index(Graph *graph, Map *map, bool *flags, int x_src, int y_src,
		 int x_dst, int y_dst)
{
	/*
	 * helper function for gen_graph_advanced
	 */

	if (map->grid[y_dst][x_dst] == '\0' && map->grid[y_src][x_src] != '#') {
		return false;
	}

	if (flags[2] && map->grid[y_dst][x_dst] == '#') {
		add_edge(graph, (pos_from_xy(map, x_src, y_src)),
			 (pos_from_xy(map, x_dst, y_dst)), 11);

	} else if (flags[0] && map->grid[y_dst][x_dst] == '~') {
		add_edge(graph, (pos_from_xy(map, x_src, y_src)),
			 (pos_from_xy(map, x_dst, y_dst)), 3);

	} else if (flags[1] && map->grid[y_dst][x_dst] == '+') {
		add_edge(graph, (pos_from_xy(map, x_src, y_src)),
			 (pos_from_xy(map, x_dst, y_dst)), 2);

	} else if (map->grid[y_dst][x_dst] != '#' &&
		   map->grid[y_dst][x_dst] != '\0') {
		add_edge(graph, (pos_from_xy(map, x_src, y_src)),
			 (pos_from_xy(map, x_dst, y_dst)), 1);
	}

	return true;
}

Graph *gen_graph_advanced(Map *map, bool *flags)
{
	/*
	 * generates adjacency list when flags are present
	 */

	Graph *graph = init_graph((map->x * map->y));

	for (int y = 0; y < map->y; y++) {
		for (int x = 0; x < map->x; x++) {
			if (map->grid[y][x] == '\0') {
				continue;
			}

			if ((x + 1) < map->x) {
				if (!check_index(graph, map, flags, x, y, x + 1,
						 y)) {
					destroy_graph(graph);
					return NULL;
				}
			}

			if ((x - 1) >= 0) {
				if (!check_index(graph, map, flags, x, y, x - 1,
						 y)) {
					destroy_graph(graph);
					return NULL;
				}
			}

			if ((y + 1) < map->y) {
				if (!check_index(graph, map, flags, x, y, x,
						 y + 1)) {
					destroy_graph(graph);
					return NULL;
				}
			}

			if ((y - 1) >= 0) {
				if (!check_index(graph, map, flags, x, y, x,
						 y - 1)) {
					destroy_graph(graph);
					return NULL;
				}
			}
		}
	}

	return graph;
}

void print_map(Map *map)
{
	/*
	 * prints the map...
	 */

	for (int y = 0; y < map->y; y++) {
		printf("%s\n", map->grid[y]);
	}
}

bool update_map(Map *map, Graph *graph, bool *flags)
{
	/*
	 * gets parent array from either BFS or Dijkstra's
	 * traverses parent array/map writing path
	 */

	int start = (map->start_y * map->x) + map->start_x;

	if (!validate_map(graph, start, map)) {
		return false;
	}

	int *parent_array = NULL;

	if (flags[0] || flags[1] || flags[2]) {
		parent_array = dijkstra(graph, start);
	} else {
		parent_array = breadth_first(graph, start);
	}

	int finish = (map->finish_y * map->x) + map->finish_x;

	int i = finish;

	i = parent_array[i];

	if (i == INT_MAX) {
		free(parent_array);
		return true;
	}

	if (flags[1]) {
		while (i != start) {
			if (map->grid[i / map->x][i % map->x] == '+' ||
			    map->grid[i / map->x][i % map->x] == '/') {
				map->grid[i / map->x][i % map->x] = '/';

			} else {
				map->grid[i / map->x][i % map->x] = '.';
			}

			i = parent_array[i];
		}
	} else {
		while (i != start) {
			map->grid[i / map->x][i % map->x] = '.';

			i = parent_array[i];
		}
	}

	free(parent_array);

	return true;
}

Map *read_maze(char *file_name, bool *flags)
{
	/*
	 * reads maze file and populates map
	 */

	FILE *fp = fopen(file_name, "r");

	if (!fp) {
		fprintf(stderr, "File: \"%s\", does not exist...\n", file_name);
		return false;
	}

	Map *map = init_map(fp);

	if (map->y < 3 || map->x < 3) {
		fprintf(stderr, "Invalid map...\n");
		free_map(map);
		fclose(fp);
		return NULL;
	}

	char *line = NULL;
	int line_num = 0;
	size_t size = 0;

	while (getline(&line, &size, fp) > 0) {
		int line_len = strlen(line);

		if (line[line_len - 1] == '\n') {
			line[line_len - 1] = '\0';
		}

		if (!validate_line(line, line_num, map, flags)) {
			fprintf(stderr, "Invalid map...\n");
			fprintf(stderr, "Invalid character in map,\n");
			fprintf(stderr, "More than one start/finish,\n");
			fprintf(stderr, "or line containing no walls...\n");
			free_map(map);
			free(line);
			fclose(fp);
			return NULL;
		}

		strcpy(map->grid[line_num], line);
		line_num++;
	}

	if (line) {
		free(line);
	}

	fclose(fp);

	return map;
}

bool validate_line(char *line, int line_num, Map *map, bool *flags)
{
	/*
	 * validates lines
	 */

	if (!strchr(line, '#')) {
		return false;
	}
	char basic[] = "#@> ";
	char water[] = "#@> ~";
	char door[] = "#@> +/";
	char both[] = "#@> ~+/";
	char *valid = basic;

	if (flags[0] && flags[1]) {
		valid = both;
	} else if (flags[0]) {
		valid = water;
	} else if (flags[1]) {
		valid = door;
	}

	for (size_t i = 0; i < strlen(line); i++) {
		if (!strchr(valid, line[i])) {
			return false;
		}

		if (line[i] == '@') {
			if (map->has_start) {
				return false;
			}
			map->start_x = i;
			map->start_y = line_num;
			map->has_start = true;
			continue;
		}

		if (line[i] == '>') {
			if (map->has_finish) {
				return false;
			}
			map->finish_x = i;
			map->finish_y = line_num;
			map->has_finish = true;
			continue;
		}
	}

	return true;
}

Map *init_map(FILE *fp)
{
	/*
	 * allocs map
	 */

	Map *map = malloc(sizeof(Map));

	get_map_size(fp, map);

	map->start_x = 0;
	map->start_y = 0;
	map->finish_x = 0;
	map->finish_y = 0;

	map->has_finish = false;
	map->has_start = false;

	map->grid = malloc(map->y * sizeof(char *));

	for (int i = 0; i < map->y; i++) {
		map->grid[i] = calloc(map->x + 1, sizeof(char));
	}

	return map;
}

void get_map_size(FILE *fp, Map *map)
{
	/*
	 * gets map x/y length
	 */

	int line_count = 0;
	int temp_count = 0;
	int char_count = 0;
	char c = '\0';

	while ((c = fgetc(fp)) != EOF) {

		if (c == '\n') {
			if (temp_count > char_count) {
				char_count = temp_count;
			}
			temp_count = 0;
			line_count++;
			continue;
		}

		temp_count++;
	}

	rewind(fp);

	map->x = char_count;
	map->y = line_count + 1;
}

void free_map(Map *map)
{
	/*
	 * frees map
	 */

	for (int i = 0; i < map->y; i++) {
		free(map->grid[i]);
	}

	free(map->grid);
	free(map);
}

void print_help(void)
{
	/*
	 * help message
	 */

	fprintf(stderr, "usage: ./maze ");
	fprintf(stderr, "[-wdD] [map filename]\n");

	fprintf(stderr,
		"w - maps can have '~' water, takes 3x as long to cross\n");
	fprintf(stderr, "d - maps can have open '/' and closed '+' doors\n");
	fprintf(stderr, "    closed doors take an extra step to cross\n");
	fprintf(stderr,
		"D - paths can go through walls, takes 11x as long to cross\n");
	fprintf(stderr, "    -D will result in MORE map restrictions...\n");
	fprintf(stderr, "    If a wall can be dug through that leads to\n");
	fprintf(stderr,
		"    A space that has a hole to the edge of the map,\n");

	fprintf(stderr, "    It could be considered an invalid map.\n");
}
