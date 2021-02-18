// based on code from
// https://www.geeksforgeeks.org/graph-and-its-representations/
// https://www.geeksforgeeks.org/dijkstras-algorithm-for-adjacency-list-representation-greedy-algo-8/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>

#include "queue.h"
#include "maze.h"
#include "min_heap.h"
#include "graph.h"

Graph *init_graph(int size)
{
	/*
	 * initializes graph
	 */

	Graph *new = malloc(sizeof(Graph));

	if (!new) {
		return NULL;
	}

	new->v = size;
	new->edge_count = 0;
	new->array = calloc(size, sizeof(Node));

	if (!new->array) {
		return NULL;
	}

	for (int i = 0; i < size; i++) {
		new->array[i] = NULL;
	}

	return new;
}

Node *create_node(int dest, int weight, int v)
{
	/*
	 * allocs new node
	 */

	Node *new = malloc(sizeof(Node));

	new->dest = dest;
	new->weight = weight;
	new->v = v;
	new->next = NULL;

	return new;
}

void add_edge(Graph *graph, int src, int dst, int weight)
{
	/*
	 * adds edge to graph
	 */

	Node *new = create_node(dst, weight, src);

	new->next = graph->array[src];
	graph->array[src] = new;
	graph->edge_count++;
}

void destroy_graph(Graph *graph)
{
	/*
	 * frees graph
	 */

	for (int i = 0; i < graph->v; i++) {
		if (graph->array[i]) {
			destroy_list(graph->array[i]);
		}
	}

	free(graph->array);
	free(graph);
}

void destroy_list(Node *node)
{
	/*
	 * frees verticies neighbors
	 */

	if (node->next) {
		destroy_list(node->next);
	}

	free(node);
}

int *breadth_first(Graph *graph, int start)
{
	/*
	 * breadth first search, generates parent array
	 */

	int *parent_array = calloc(graph->v, sizeof(int));

	for (int i = 0; i < graph->v; i++) {
		parent_array[i] = INT_MAX;
	}

	int *visited = calloc(graph->v, sizeof(int));

	int count = 0;

	Queue *q = queue_create(graph->edge_count);

	queue_enqueue(q, graph->array[start]);

	visited[start] = 1;

	Node *temp = NULL;

	while (!queue_empty(q)) {
		temp = queue_front(q);
		queue_dequeue(q);

		while (temp) {
			count++;
			if (!visited[temp->dest]) {
				parent_array[temp->dest] = temp->v;
				queue_enqueue(q, graph->array[temp->dest]);
				visited[temp->dest] = 1;
			}
			temp = temp->next;
		}
	}

	free(visited);
	queue_destroy(q);

	return parent_array;
}

bool validate_map(Graph *graph, int start, Map *map)
{
	/*
	 * BFS to find holes in map
	 */

	int *visited = calloc(graph->v, sizeof(int));

	int count = 0;

	Queue *q = queue_create(graph->edge_count);

	queue_enqueue(q, graph->array[start]);

	visited[start] = 1;

	while (!queue_empty(q)) {
		Node *temp = queue_front(q);

		queue_dequeue(q);

		while (temp) {
			if (!is_in_bounds(temp->v, temp->dest, map)) {
				free(visited);
				queue_destroy(q);
				return false;
			}
			count++;
			if (!visited[temp->dest]) {
				queue_enqueue(q, graph->array[temp->dest]);
				visited[temp->dest] = 1;
			}
			temp = temp->next;
		}
	}

	free(visited);
	queue_destroy(q);

	return true;
}

void decrease_key(Heap *heap, int v, int dist)
{
	/*
	 * Decreases distance for vertex
	 */

	int i = heap->pos[v];

	heap->array[i]->dist = dist;

	while (i && heap->array[i]->dist < heap->array[(i - 1) / 2]->dist) {
		heap->pos[heap->array[i]->v] = (i - 1) / 2;

		heap->pos[heap->array[(i - 1) / 2]->v] = i;

		Item *temp = NULL;

		temp = heap->array[i];
		heap->array[i] = heap->array[(i - 1) / 2];
		heap->array[(i - 1) / 2] = temp;

		i = (i - 1) / 2;
	}
}

int *dijkstra(Graph *graph, int start)
{
	/*
	 * Dijkstra's algo for shortest path
	 */

	int V = graph->v;

	int *dist = malloc(V * sizeof(int));

	int *parent_array = calloc(graph->v, sizeof(int));

	for (int i = 0; i < graph->v; i++) {
		parent_array[i] = INT_MAX;
	}

	Heap *heap = create_heap(V);

	for (int v = 0; v < V; v++) {
		dist[v] = INT_MAX;
		heap->array[v] = create_item(v, dist[v]);
		heap->pos[v] = v;
	}

	heap->pos[start] = start;

	dist[start] = 0;

	decrease_key(heap, start, dist[start]);

	heap->count = V;

	while (heap->count > 0) {
		Item *h_node = NULL;

		h_node = heap_extract(heap);

		int u = h_node->v;

		Node *crawl = graph->array[u];

		while (crawl) {
			int v = crawl->dest;

			if (dist[u] != INT_MAX &&
			    crawl->weight + dist[u] < dist[v]) {
				dist[v] = dist[u] + crawl->weight;
				parent_array[crawl->dest] = crawl->v;
				decrease_key(heap, v, dist[v]);
			}

			crawl = crawl->next;
		}

		free(h_node);
	}

	heap_destroy(heap);
	free(dist);

	return parent_array;
}
