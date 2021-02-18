// get_distance function based on:
// www.b4x.com/android/forum/threads/distance-between-two-gps-points-with-altitude.72072/
// www.geeksforgeeks.org/haversine-formula-to-find-distance-between-two-points-on-a-sphere/

/*
 * Usage: ./zergmap [-h:num -n:num] [pcap1] [pcap2] ....
 * -h print out any zerg IDs that are below num% health, rather than the
 *    default of 10%
 * -n limits the number of changes to no more than num
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "decode.h"
#include "graph.h"
#include "zergmap.h"

int main(int argc, char **argv)
{
	double health = .10;
	int del_limit = -1;

	handle_args(argc, argv, &health, &del_limit);

	Zergs *zergs = init_zergs(10);

	for (int i = optind; i < argc; i++) {
		if (!read_pcap(argv[i], zergs)) {
			free_zergs(zergs);
			return 1;
		}
	}

	cleanup_zergs(zergs);

	if (zergs->count == 0) {
		if (zergs->start_size == 0) {
			fprintf(stderr, "No zergs in pcap(s)..\n");
			free(zergs);
			return 1;
		}
		print_health(zergs, health);
		print_no_gps(zergs);
		free_zergs(zergs);
		return 1;
	}

	Graph *graph = create_graph(zergs);

	if (del_limit > -1) {
		graph->del_limit = del_limit;
	}

	do {
		graph = remove_components(graph, zergs);

		if (!graph) {
			printf("TOO MANY CHANGES REQUIRED\n");
			break;
		}

		graph = prune_graph(graph, zergs);

		if (!graph) {
			printf("TOO MANY CHANGES REQUIRED\n");
			break;
		}

		if (graph->v > 2) {
			graph = get_nodes_to_kill(graph, zergs);

			if (!graph) {
				printf("TOO MANY CHANGES REQUIRED\n");
				break;
			}
		}

		if (graph->has_too_close) {
			graph = collision(graph, zergs);

			if (!graph) {
				printf("TOO MANY CHANGES REQUIRED\n");
				break;
			}
		}

	} while (!is_fully_connected(graph));

	if (graph) {
		print_dead(zergs);
	}

	print_health(zergs, health);
	print_no_gps(zergs);
	free_zergs(zergs);

	if (graph) {
		destroy_graph(graph);
	}

	return 0;
}

void handle_args(int argc, char **argv, double *health, int *del_limit)
{
	/*
	 * Checks arguments and sets health/del_limit if flags are present
	 */

	int opt = 0;
	int temp = 0;

	while ((opt = getopt(argc, argv, "h:n:")) != -1) {

		switch (opt) {
		case 'h':
			if (!isdigit(optarg[0])) {
				print_help();
				exit(1);
			}

			temp = atoi(optarg);

			if (temp < 0) {
				print_help();
				exit(1);
			} else {
				*health = (double)temp / 100;
			}

			break;
		case 'n':
			if (!isdigit(optarg[0])) {
				print_help();
				exit(1);
			}

			temp = atoi(optarg);

			if (temp < 0) {
				print_help();
				exit(1);
			} else {
				*del_limit = temp;
			}

			break;
		case '?':
			print_help();
			exit(1);
		}
	}

	// print help if no arguments are leftover
	if ((argc - optind) < 1) {
		print_help();
		exit(1);
	}
}

void print_no_gps(Zergs *zergs)
{
	/*
	 * Prints zerg ids of zergs missing gps information
	 */

	uint16_t *ids = calloc(zergs->start_size, sizeof(uint16_t));
	int count = 0;

	for (int i = 0; i < zergs->start_size; i++) {
		if (!zergs->zergs[i]->has_gps) {
			ids[count] = zergs->zergs[i]->id;
			count++;
		}
	}

	if (count == 0) {
		free(ids);
		return;
	}

	printf("Missing GPS:\n");
	for (int i = 0; i < count; i++) {
		printf("zerg #%d\n", ids[i]);
	}

	free(ids);
}

void print_health(Zergs *zergs, double health)
{
	/*
	 * Prints zerg ids of zergs whose health is below the threshold (health)
	 */

	uint16_t *ids = calloc(zergs->start_size, sizeof(uint16_t));
	int count = 0;

	for (int i = 0; i < zergs->start_size; i++) {
		if (((double)zergs->zergs[i]->health /
		     zergs->zergs[i]->max_health) < health) {
			ids[count] = zergs->zergs[i]->id;
			count++;
		}
	}

	if (count == 0) {
		free(ids);
		return;
	}

	printf("Low Health:\n");
	for (int i = 0; i < count; i++) {
		printf("zerg #%d\n", ids[i]);
	}

	free(ids);
}

void print_dead(Zergs *zergs)
{
	/*
	 * Prints zerg ids of zergs deleted from the graph
	 */

	if (zergs->del_count == 0) {
		puts("ALL ZERGS ARE IN POSITION");
		return;
	}

	printf("Network Alterations:\n");

	for (int i = 0; i < zergs->start_size; i++) {
		if (zergs->zergs[i]->deleted) {
			printf("Remove zerg #%d\n", zergs->zergs[i]->id);
		}
	}
}

void cleanup_zergs(Zergs *zergs)
{
	/*
	 * Soft deletes zergs with missing gps data
	 */

	if (zergs->count == 0) {
		return;
	}

	Zerg *zerg = NULL;

	zergs->start_size = zergs->count;

	// moves zergs to make of array and decrements count
	for (int i = zergs->count - 1; i >= 0; i--) {
		zerg = zergs->zergs[i];
		if (!zerg->has_gps) {
			zerg = zergs->zergs[i];
			zergs->zergs[i] = zergs->zergs[zergs->count - 1];
			zergs->zergs[zergs->count - 1] = zerg;

			zergs->count--;
		}
	}
}

void print_dists(Zergs *zergs)
{
	/*
	 * Prints distances between zergs
	 */

	Zerg *zerg = NULL;

	for (int i = 0; i < zergs->count; i++) {
		zerg = zergs->zergs[i];
		for (int j = 0; j < zergs->count; j++) {
			if (j != i) {
				printf("Distance from %d to %d -> %0.2fm\n",
				       zerg->id, zergs->zergs[j]->id,
				       get_distance(zerg, zergs->zergs[j]));
			}
		}

		puts("");
	}
}

Zergs *init_zergs(int size)
{
	/*
	 * Allocs zergs array
	 */

	Zergs *new = malloc(sizeof(*new));

	if (!new) {
		return NULL;
	}

	new->count = 0;
	new->max = size;
	new->start_size = 0;
	new->del_count = 0;

	new->zergs = calloc(new->max, sizeof(new->zergs));

	if (!new->zergs) {
		free(new);
		return NULL;
	}

	return new;
}

void free_zergs(Zergs *zergs)
{
	/*
	 * Frees zergs array
	 */

	int size = 0;

	if (zergs->count > zergs->start_size) {
		size = zergs->count;
	} else {
		size = zergs->start_size;
	}

	for (int i = 0; i < size; i++) {
		free(zergs->zergs[i]);
	}

	free(zergs->zergs);
	free(zergs);
}

void update_zerg_health(Zerg *new, Zerg *old)
{
	/*
	 * Updates zerg health
	 */

	old->health = new->health;
	old->max_health = new->max_health;
}

bool update_zerg_gps(Zerg *new, Zerg *old)
{
	/*
	 * Updates zerg gps information
	 */

	if (get_distance(new, old) > old->acc) {
		fprintf(stderr, "Conflicting GPS Coordinates ");
		fprintf(stderr, "For Zerg #%d..\n", new->id);
		return false;
	}

	old->lat = new->lat;
	old->lon = new->lon;
	old->alt = new->alt;
	old->acc = new->acc;

	return true;
}

Zerg *zerg_exists(Zergs *zergs, Zerg *zerg)
{
	/*
	 * Checks if zerg exists
	 */

	for (int i = 0; i < zergs->count; i++) {
		if (zerg->id == zergs->zergs[i]->id) {
			return zergs->zergs[i];
		}
	}

	return NULL;
}

bool add_zerg(Zergs *zergs, Zerg *zerg, bool is_gps)
{
	/*
	 * Adds zerg to zergs array
	 */

	Zerg *exists = zerg_exists(zergs, zerg);

	if (exists) {
		if (is_gps) {
			if (!update_zerg_gps(zerg, exists)) {
				// bad distance accuracy difference
				free(zerg);
				return false;
			}

			free(zerg);
			return true;
		}

		update_zerg_health(zerg, exists);
		free(zerg);
		return true;
	}

	double count = zergs->count;
	double size = zergs->max;

	if (((double)count / size) > .79) {
		zergs->zergs =
		    realloc(zergs->zergs, (zergs->max * 2) * sizeof(Zerg *));

		zergs->max = zergs->max * 2;
	}

	if (is_gps) {
		zerg->has_gps = true;
	}

	zergs->zergs[zergs->count] = zerg;
	zergs->count++;

	return true;
}

Zerg *create_zerg(uint16_t id)
{
	/*
	 * Allocs zerg
	 */

	Zerg *new_zerg = malloc(sizeof(Zerg));

	new_zerg->lat = 0;
	new_zerg->lon = 0;
	new_zerg->alt = 0;
	new_zerg->acc = 0;
	new_zerg->health = 0;
	new_zerg->max_health = 0;
	new_zerg->has_gps = false;
	new_zerg->deleted = false;
	new_zerg->id = id;

	return new_zerg;
}

double get_distance(Zerg *zerg1, Zerg *zerg2)
{
	/*
	 * Gets distance between two zergs
	 */

	double lat1 = zerg1->lat;
	double lon1 = zerg1->lon;
	double alt1 = zerg1->alt;

	double lat2 = zerg2->lat;
	double lon2 = zerg2->lon;
	double alt2 = zerg2->alt;

	// distance between latitudes
	// and longitudes
	double dLat = (lat2 - lat1) * M_PI / 180.0;
	double dLon = (lon2 - lon1) * M_PI / 180.0;

	// convert to radians
	lat1 = (lat1)*M_PI / 180.0;
	lat2 = (lat2)*M_PI / 180.0;

	// apply formulae
	double a = pow(sin(dLat / 2), 2) +
		   pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
	double rad = 6371;
	double c = 2 * asin(sqrt(a));

	double distance = (rad * c) * 1000;

	distance = sqrt(pow(distance, 2) + pow(alt1 - alt2, 2));

	return distance;
}

Graph *get_nodes_to_kill(Graph *graph, Zergs *zergs)
{
	/*
	 * Calls determine_bad_nodes and deletes the verticies in the array it
	 * returns
	 */

	int *kills = determine_bad_nodes(graph);

	// if an array was returned
	if (kills) {
		// if node not marked with a two (2) - delete it
		for (int i = graph->v - 1; i >= 0; i--) {
			if (kills[i] != 2) {
				del_node(graph, i, zergs);
			}
		}

		free(kills);

		// ensure that the number of nodes deleted is not
		// above the delete limit
		if ((graph->start_v - graph->v) > graph->del_limit) {
			destroy_graph(graph);
			return NULL;
		}

		return graph;
	}

	destroy_graph(graph);
	return NULL;
}

void print_help(void)
{
	fprintf(stderr,
		"Usage: ./zergmap [-h:num -n:num] [pcap1] [pcap2] ....\n");
	fprintf(stderr, "-h print out any zerg IDs that are below num%% ");
	fprintf(stderr, "health,\n   rather than the default of 10%%.\n");
	fprintf(stderr,
		"-n limits the number of changes to no more than num\n");
}
