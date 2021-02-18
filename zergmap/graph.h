#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Zergs Zergs;

typedef struct Node {
	int dest;
	double weight;
	int v;
	struct Node *next;
} Node;

typedef struct Graph {
	int v;
	int start_v;
	struct Node **array;
	int edge_count;
	bool has_too_close;
	int del_limit;
} Graph;

Graph *init_graph(int size);
Graph *create_graph(Zergs *zergs);
Node *create_node(int dest, double weight, int v);
void add_edge(Graph *graph, int src, int dest, int weight);
void print_graph(Graph *graph, Zergs *zergs);

Graph *remove_components(Graph *graph, Zergs *zergs);
Graph *prune_graph(Graph *graph, Zergs *zergs);

void update_index(Graph *graph, int new, int old);

int *dijkstra(Graph *graph, int start);
void print_path(int *p, int size, int start, Zergs *zergs);

bool is_neighbor(Graph *graph, int src, int dst);
void copy_graph(Graph *graph1, Graph *graph2);

bool baby_suurballe(Graph *graph1, int src, int dst);

void remove_edges(Graph *graph, int src, int dst, int *parent);
void remove_nodes(Graph *graph, int src, int *dst, int *parent);

bool is_fully_connected(Graph *graph);

void destroy_graph(Graph *graph);
void destroy_list(Node *node);
Graph *collision(Graph *graph, Zergs *zergs);
void del_node(Graph *graph, int index, Zergs *zergs);

void set_zero(int *array, int size);
int *determine_bad_nodes(Graph *graph);
int *get_kill_array(Graph *graph);
int first_cycle(Graph *graph, int start, int end, int *touched);
int attach_to_cycle(Graph *graph, int *touched);
void cp_array(int *source_array, int *dest_array, int size);
int in_cycle(Graph *graph, int snode, int *touched);
int min_cycle(Graph *graph, int snode, int enode, int *touched);
