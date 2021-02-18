#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
	int dest;
	int weight;
	int v;
	struct Node *next;
} Node;

typedef struct Graph {
	int v;
	int edge_count;
	struct Node **array;
} Graph;

typedef struct Map Map;
typedef struct Heap Heap;

Graph *init_graph(int size);
Node *create_node(int dest, int weight, int v);
void add_edge(Graph *graph, int src, int dst, int weight);
void destroy_graph(Graph *graph);
void destroy_list(Node *node);
int *breadth_first(Graph *graph, int start);
bool validate_map(Graph *graph, int start, Map *map);
void decrease_key(Heap *heap, int v, int dist);
int *dijkstra(Graph *graph, int start);