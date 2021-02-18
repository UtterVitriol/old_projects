// inspiration from https://www.geeksforgeeks.org/graph-and-its-representations/
// https://www.geeksforgeeks.org/dijkstras-algorithm-for-adjacency-list-representation-greedy-algo-8/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>

#include "min_heap.h"
#include "queue.h"
#include "zergmap.h"
#include "graph.h"

// max distance allowed between zergs to for connection
const double MAX_DIST = 15.00000;

// min distance allowed between zergs to for connection
const double MIN_DIST = 1.14300;

Graph *init_graph(int size)
{
	/*
	 * Allocs graph
	 */

	Graph *new = malloc(sizeof(Graph));

	new->v = size;
	new->start_v = size;
	new->array = calloc(size, sizeof(Node));
	new->edge_count = 0;
	new->has_too_close = false;
	new->del_limit = (double)new->start_v / 2;

	for (int i = 0; i < size; i++) {
		new->array[i] = NULL;
	}

	return new;
}

Graph *create_graph(Zergs *zergs)
{
	/*
	 * Creates and populates graph of zergs
	 */

	Graph *graph = NULL;

	graph = init_graph(zergs->count);

	double dist = 0;

	Zerg *zerg1 = NULL;
	Zerg *zerg2 = NULL;

	// add edges to graph if zergs are within MAX_DIST/MIN_DIST
	for (int i = 0; i < zergs->count; i++) {
		zerg1 = zergs->zergs[i];
		for (int j = i + 1; j < zergs->count; j++) {
			zerg2 = zergs->zergs[j];
			dist = get_distance(zerg1, zerg2);

			if (dist <= MAX_DIST && dist >= MIN_DIST) {
				add_edge(graph, i, j, 1);
				add_edge(graph, j, i, 1);
			}

			if (dist < MIN_DIST) {
				graph->has_too_close = true;
			}
		}
	}

	return graph;
}

void remove_neighbor(Graph *graph, int src, int dest)
{
	/*
	 * Removes edge in graph
	 */

	Node *node = graph->array[src];
	Node *last = NULL;

	while (node) {
		if (node->dest == dest) {
			if (last) {
				last->next = node->next;
				free(node);
				return;
			}

			graph->array[src] = node->next;
			free(node);
			return;
		}

		last = node;
		node = node->next;
	}
}

void swap_nodes(Graph *graph, Zergs *zergs, int index)
{
	/*
	 * Soft deletes vertex in graph
	 */

	Node *node = NULL;
	Zerg *zerg = NULL;

	if (zergs) {
		zerg = zergs->zergs[index];
		zergs->zergs[index] = zergs->zergs[graph->v - 1];
		zergs->zergs[graph->v - 1] = zerg;
	}

	if (graph) {
		node = graph->array[index];
		graph->array[index] = graph->array[graph->v - 1];
		graph->array[graph->v - 1] = node;
	}

	update_index(graph, index, graph->v - 1);
}

void update_index(Graph *graph, int new, int old)
{
	/*
	 * Updates edges in graph for new index of vertex
	 */

	Node *node = NULL;

	for (int i = 0; i < graph->v; i++) {
		node = graph->array[i];
		while (node) {
			if (node->dest == old) {
				node->dest = new;
			}
			node = node->next;
		}
	}

	node = graph->array[new];

	while (node) {
		node->v = new;
		node = node->next;
	}
}

Graph *prune_graph(Graph *graph, Zergs *zergs)
{
	/*
	 * Removes leaf/singleton verticies in graph
	 */

	for (int i = graph->v - 1; i >= 0; i--) {

		if ((graph->start_v - graph->v) > graph->del_limit) {
			destroy_graph(graph);
			return NULL;
		}

		if (graph->v <= 2) {
			return graph;
		}

		if (!graph->array[i]) {
			zergs->zergs[i]->deleted = true;
			zergs->del_count++;
			swap_nodes(graph, zergs, i);
			graph->v--;
			continue;
		}

		if (!graph->array[i]->next) {

			zergs->zergs[i]->deleted = true;
			zergs->del_count++;
			remove_neighbor(graph, graph->array[i]->dest, i);
			swap_nodes(graph, zergs, i);
			graph->v--;
		}
	}

	return graph;
}

Node *create_node(int dest, double weight, int v)
{
	/*
	 * Allocs edge for graph
	 */

	Node *new = malloc(sizeof(Node));

	new->dest = dest;
	new->weight = weight;
	new->v = v;
	new->next = NULL;

	return new;
}

void add_edge(Graph *graph, int src, int dest, int weight)
{
	/*
	 * Adds edge to graph
	 */

	Node *new = create_node(dest, weight, src);

	new->next = graph->array[src];
	graph->array[src] = new;
	graph->edge_count++;
}

void destroy_graph(Graph *graph)
{
	/*
	 * Frees graph
	 */

	for (int i = 0; i < graph->start_v; i++) {
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
	 * Frees vertex edges in graph
	 */

	if (node->next) {
		destroy_list(node->next);
	}

	free(node);
}

void print_graph(Graph *graph, Zergs *zergs)
{
	/*
	 * Prints the graph
	 */

	Node *node = NULL;
	int v = 0;

	for (; v < graph->v; v++) {
		node = graph->array[v];

		if (node) {
			printf("Neighbors of %d: ", zergs->zergs[v]->id);

			while (node) {
				printf("%d, ", zergs->zergs[node->dest]->id);

				node = node->next;
			}

			puts("");
		}
	}

	printf("V: %d\n", graph->v);
}

void print_path(int *p, int size, int start, Zergs *zergs)
{
	/*
	 * Prints shortest paths for vertex
	 */

	printf("%d\n", zergs->zergs[start]->id);

	int temp = 0;

	for (int i = 0; i < size; i++) {
		if (i == start) {
			continue;
		}

		temp = p[i];

		if (temp == INT_MAX) {
			continue;
		}

		printf("%d <- ", zergs->zergs[i]->id);

		while (temp != start) {
			printf("%d <- ", zergs->zergs[temp]->id);
			temp = p[temp];
		}

		printf("%d\n", zergs->zergs[start]->id);
	}
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
	 * Dijkstra's shortest path
	 */

	int V = graph->v;

	int *dist = malloc(V * sizeof(int));

	int *parent_array = calloc(V, sizeof(int));

	for (int i = 0; i < V; i++) {
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
	parent_array[start] = start;

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

bool is_neighbor(Graph *graph, int src, int dst)
{
	/*
	 * Checks if edge exists between two vertecies
	 */

	Node *node = graph->array[src];

	while (node) {
		if (node->dest == dst) {
			return true;
		}
		node = node->next;
	}

	return false;
}

void copy_graph(Graph *graph1, Graph *graph2)
{
	/*
	 * Copies graph1 to graph2
	 */

	Node *node = NULL;

	for (int i = 0; i < graph1->v; i++) {
		node = graph1->array[i];

		while (node) {
			add_edge(graph2, i, node->dest, 1);
			node = node->next;
		}
	}
}

void remove_edges(Graph *graph, int src, int dst, int *parent)
{
	/*
	 * Walks parent array and deletes the edges from graph
	 */

	int temp_d = dst;
	int temp_s = 0;

	while (true) {
		temp_s = parent[temp_d];

		remove_neighbor(graph, temp_s, temp_d);

		if (temp_s == src) {
			return;
		}

		temp_d = temp_s;
	}
}

void remove_nodes(Graph *graph, int src, int *dst, int *parent)
{
	/*
	 * Walks parent array and deletes the vertecies from graph
	 */

	int temp_d = *dst;
	int temp_s = 0;

	int *to_remove = malloc(graph->v * sizeof(int));
	int count = 0;

	while (true) {
		temp_s = parent[temp_d];

		if (temp_s == src) {
			free(to_remove);
			return;
		}

		to_remove[count] = temp_s;
		count++;

		temp_d = temp_s;
	}

	for (int i = 0; i < count; i++) {
		del_node(graph, to_remove[count], NULL);
	}

	free(to_remove);
}

void del_node(Graph *graph, int index, Zergs *zergs)
{
	/*
	 * Deletes node from graph and updates index in zergs
	 */

	Node *node = NULL;
	Node *temp = NULL;
	Zerg *zerg = NULL;

	node = graph->array[index];

	while (node) {
		temp = node->next;
		remove_neighbor(graph, node->dest, index);
		node = temp;
	}

	if (zergs) {
		zergs->zergs[index]->deleted = true;
		zergs->del_count++;

		zerg = zergs->zergs[index];
		zergs->zergs[index] = zergs->zergs[graph->v - 1];
		zergs->zergs[graph->v - 1] = zerg;
	}

	node = graph->array[index];
	graph->array[index] = graph->array[graph->v - 1];
	graph->array[graph->v - 1] = node;

	update_index(graph, index, graph->v - 1);

	graph->v--;
}

bool baby_suurballe(Graph *graph1, int src, int dst)
{
	/*
	 * Determines if there are two node disjoint paths between any two nodes
	 */

	int *parent1 = NULL;
	int *parent2 = NULL;
	int *parent3 = NULL;
	int *parent4 = NULL;

	parent1 = dijkstra(graph1, src);

	if (parent1[dst] == INT_MAX) {
		free(parent1);
		return false;
	}

	Graph *graph2 = init_graph(graph1->v);

	copy_graph(graph1, graph2);

	remove_edges(graph2, src, dst, parent1);

	// checks for singleton nodes
	for (int i = 0; i < graph2->v; i++) {
		if (!graph2->array[i]) {
			destroy_graph(graph2);
			free(parent1);
			return false;
		}
	}

	parent2 = dijkstra(graph2, src);

	if (parent2[dst] == INT_MAX) {
		destroy_graph(graph2);
		free(parent1);
		free(parent2);

		return false;
	}

	destroy_graph(graph2);

	Graph *graph3 = init_graph(graph1->v);

	copy_graph(graph1, graph3);

	// remove common edges
	for (int i = 0; i < graph3->v; i++) {
		if (i == parent2[parent1[i]]) {
			remove_neighbor(graph3, i, parent2[parent1[i]]);
			remove_neighbor(graph3, parent2[parent1[i]], i);
		}
	}

	free(parent1);
	free(parent2);

	parent3 = dijkstra(graph3, src);

	if (parent3[dst] == INT_MAX) {
		destroy_graph(graph3);
		free(parent3);

		return false;
	}

	remove_nodes(graph3, src, &dst, parent3);
	free(parent3);

	parent4 = dijkstra(graph3, src);

	if (parent4[dst] == INT_MAX) {
		destroy_graph(graph3);
		free(parent4);

		return false;
	}

	destroy_graph(graph3);
	free(parent4);

	return true;
}

bool is_fully_connected(Graph *graph)
{
	/*
	 * Runs suurballes between every vertex and every other vertext that's
	 * not directly adjacent to it in order to determine if graph
	 * is "fully connected"
	 */

	for (int i = 0; i < graph->v; i++) {
		for (int j = i + 1; j < graph->v; j++) {
			if (!is_neighbor(graph, i, j)) {
				if (!baby_suurballe(graph, i, j)) {
					return false;
				}
			}
		}
	}

	return true;
}

int *breadth_first(Graph *graph, int start)
{
	/*
	 * breadth first search
	 */

	int *parent_array = calloc(graph->v, sizeof(int));

	for (int i = 0; i < graph->v; i++) {
		parent_array[i] = INT_MAX;
	}

	int *visited = calloc(graph->v, sizeof(int));

	int count = 0;

	Queue *q = queue_create(graph->edge_count + 2);

	queue_enqueue(q, graph->array[start]);

	visited[start] = 1;
	parent_array[start] = start;

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

Graph *remove_components(Graph *graph, Zergs *zergs)
{
	/*
	 * Removes components from graph
	 */

	bool *visited = calloc(graph->start_v, sizeof(bool));
	bool found_fifty = false;
	int *parent = NULL;
	int count = 0;

	// runs breadth first on every node that hasn't been visited
	for (int i = 0; i < graph->v; i++) {
		if ((graph->start_v - graph->v) > graph->del_limit) {
			destroy_graph(graph);
			return NULL;
		}

		if (visited[i]) {
			continue;
		}

		parent = breadth_first(graph, i);

		for (int j = 0; j < graph->v; j++) {
			if (parent[j] != INT_MAX) {
				visited[j] = true;
				count++;
			}
		}

		// delete nodes from graph if they aren't in a component of at
		// least half the total number of nodes
		if (((double)count / graph->start_v) < .50 || found_fifty) {

			for (int k = graph->v - 1; k >= 0; k--) {
				if (parent[k] != INT_MAX) {
					zergs->zergs[k]->deleted = true;
					zergs->del_count++;
					swap_nodes(graph, zergs, k);
					graph->v--;
				}
			}
		} else {

			found_fifty = true;
		}

		// visited all nodes.
		if (count == graph->v) {
			free(visited);
			free(parent);
			return graph;
		}

		count = 0;

		free(parent);
	}

	free(visited);
	return graph;
}

int *determine_bad_nodes(Graph *graph)
{
	/*
	 * Function returns an array. That array notes the nodes that are
	 * in a cycle with a two (2) - all other nodes should be deleted
	 */

	int cycle = 0;
	int cap = 0;
	double size = 0;
	int *touched = (int *)calloc(sizeof(int), graph->v);

	// continue until every node has been attempted
	while (cap < graph->v) {
		size = 0;
		for (int i = cap; i < graph->v; i++) {
			set_zero(touched, graph->v);

			Node *node = graph->array[i];

			// for eahc neighbor
			while (node) {
				// find a cycle between the node and its
				// neighbor
				cycle = first_cycle(graph, node->v, node->dest,
						    touched);
				if (cycle) {
					break;
				}

				node = node->next;
			}
			if (cycle)
				break;
		}

		// attempt to attach every node to the cycle
		attach_to_cycle(graph, touched);

		// count how many nodes are in the resulting cycle
		for (int i = 0; i < graph->v; i++) {
			if (touched[i] == 2) {
				++size;
			}
		}

		// if at least half the possible nodes are in the cycle, break
		if (size * 2 >= graph->v) {
			break;
		}
		++cap;
	}

	return touched;
}

int first_cycle(Graph *graph, int start, int end, int *touched)
{
	/*
	 * Gets first cycle
	 */

	for (int i = 0; i < graph->v; i++) {
		touched[i] = 0;
	}

	touched[end] = 2;

	int cycle = min_cycle(graph, start, end, touched);

	if (!cycle) {
		touched[start] = 0;
		touched[end] = 0;
	}
	for (int i = 0; i < graph->v; i++) {
		if (touched[i] != 2)
			touched[i] = 0;
	}

	return cycle;
}

int min_cycle(Graph *graph, int snode, int enode, int *touched)
{
	/*
	 * Determine the minimum path between a node and its neighbor,
	 * without traversing directly to it - this means there is a cycle
	 */

	Heap *heap = create_heap(graph->edge_count);

	int *weight = (int *)calloc(sizeof(int), graph->v);

	int *parent = (int *)calloc(sizeof(int), graph->v);

	Node *cnode = NULL;

	// set weights
	for (int i = 0; i < graph->v; i++) {
		weight[i] = INT_MAX;
		parent[i] = i;
	}

	weight[snode] = 0;

	heap_insert(heap, snode, weight[snode]);

	int cur_key;

	// continue until the heap is empty
	while (heap->count > 0) {

		Item *h_node = NULL;

		h_node = heap_extract(heap);

		cur_key = h_node->v;

		if (touched[cur_key]) {
			free(h_node);
			continue;
		}

		cnode = graph->array[cur_key];

		// for each neighbor
		while (cnode) {

			touched[cur_key] = 1;
			int v = cur_key;

			if (!touched[cnode->dest]) {
				// check if this node has a
				// lower weight
				if (weight[cnode->dest] >
				    (weight[v] + cnode->weight)) {
					parent[cnode->dest] = v;
					weight[cnode->dest] =
					    weight[v] + cnode->weight;

					heap_insert(heap, cnode->dest,
						    weight[cnode->dest]);
				}
			} else if (touched[cnode->dest] == 2 &&
				   cnode->dest == enode && v != snode) {
				if (weight[cnode->dest] >
				    (weight[v] + cnode->weight)) {
					parent[cnode->dest] = v;
					weight[cnode->dest] =
					    weight[v] + cnode->weight;

					heap_insert(heap, cnode->dest,
						    weight[cnode->dest]);
				}
			}

			cnode = cnode->next;
		}

		free(h_node);
	}

	// find if there is a path
	int cur = enode;
	int path_exists = 1;

	while (cur != snode) {
		if (cur == parent[cur]) {
			path_exists = 0;
			break;
		}
		cur = parent[cur];
	}

	cur = enode;

	// if there is a path, note that path
	if (path_exists) {
		while (cur != snode) {
			touched[cur] = 2;
			cur = parent[cur];
		}
		touched[cur] = 2;
	}

	heap_destroy(heap);
	free(weight);
	free(parent);
	if (path_exists)
		return 1;
	else
		return 0;
}

int attach_to_cycle(Graph *graph, int *touched)
{
	/*
	 * function used to find two entries into the cycle
	 * if this is found, add the node to the cycle
	 */

	int *new_touched = (int *)calloc(sizeof(int), graph->v);
	int attached;
	int entries;
	int cycle_entry;
	Node *node = NULL;
	Node *temp = NULL;

	for (int i = 0; i < graph->v; i++) {
		attached = -1;
		entries = 0;
		cycle_entry = -1;

		// loop through nodes again

		if (touched[i] != 2) {
			node = graph->array[i];

			// for each neighbor, can the node enter the cycle
			while (node) {
				cp_array(touched, new_touched, graph->v);

				temp = graph->array[node->dest];

				// if it can hit the cycle at least once,
				// increase the count ("entries") and note the
				// entry point
				if (touched[temp->v] == 2 &&
				    temp->v != cycle_entry) {
					entries++;
					cycle_entry = temp->v;
				} else {
					attached = in_cycle(graph, temp->v,
							    new_touched);
					// if it enters the cycle through a
					// second node increase the count
					// ("entries")
					if (attached != cycle_entry &&
					    attached != -1) {
						entries++;
						cycle_entry = attached;
					}
				}

				node = node->next;
			}
		}

		// if there are at least two entries, add to the cycle
		if (entries > 1) {
			touched[i] = 2;
			touched[temp->v] = 2;
		}
	}
	free(new_touched);
	return 1;
}

void cp_array(int *source_array, int *dest_array, int size)
{
	/*
	 * Copies source array to dest array
	 */

	for (int i = 0; i < size; i++)
		dest_array[i] = source_array[i];
}

int in_cycle(Graph *graph, int snode, int *touched)
{
	/*
	 *Determine the minimum path between two nodes, and return whether
	 *a cycle was found
	 */

	Heap *heap = create_heap(graph->v);

	int *weight = (int *)calloc(sizeof(int), graph->v);
	int *parent = (int *)calloc(sizeof(int), graph->v);

	Node *cnode = NULL;
	Node *nnode = NULL;

	// set weights
	for (int i = 0; i < graph->v; i++) {
		weight[i] = INT_MAX;
		parent[i] = graph->array[i]->v;
	}

	weight[snode] = 0;

	heap_insert(heap, snode, weight[snode]);

	int cur_key;

	// continue until the heap is empty
	while (heap->count > 0) {

		Item *h_node = NULL;

		h_node = heap_extract(heap);

		cur_key = h_node->v;

		if (!touched[cur_key]) { // only entered if node has not
					 // already been itterated through
			touched[cur_key] = 1;
			cnode = graph->array[cur_key];

			while (cnode) { // loop through the neighbors
				nnode = graph->array[cnode->dest];

				if (!touched[nnode->v]) {
					// check if this node has a
					// lower weight
					if (weight[nnode->v] >
					    (weight[cnode->v] +
					     cnode->weight)) {
						// if so, change parent and add
						// to heap
						parent[nnode->v] = cnode->v;
						weight[nnode->v] =
						    weight[cnode->v] +
						    cnode->weight;

						heap_insert(heap, nnode->v,
							    weight[nnode->v]);
					}
				} else if (touched[nnode->v] == 2) {
					// otherwise, if the cycle was found,
					// return
					free(h_node);
					heap_destroy(heap);
					free(weight);
					free(parent);
					return nnode->v;
				}

				cnode = cnode->next;
			}
		}

		free(h_node);
	}

	// only reached when no cycle was found
	heap_destroy(heap);
	free(weight);
	free(parent);
	return -1;
}

Graph *collision(Graph *graph, Zergs *zergs)
{
	/* Function that handles the zerg nodes that are too close together.
	 */

	for (int x = graph->v - 1; x > 0; x--) {
		for (int y = x - 1; y >= 0; y--) {
			// Finds nodes that are too close
			if (get_distance(zergs->zergs[x], zergs->zergs[y]) <
			    MIN_DIST) {
				// Creates two copy graphs that check removals
				// required
				Graph *xless = init_graph(graph->v);
				Graph *yless = init_graph(graph->v);

				copy_graph(graph, xless);
				copy_graph(graph, yless);
				del_node(xless, x, NULL);
				del_node(yless, y, NULL);

				// Too few nodes for 'get_nodes_to_kill'
				if (xless->v <= 2) {
					if (xless->array[0]->dest == 1) {
						swap_nodes(graph, zergs, x);
					} else if (yless->array[0]->dest == 1) {
						swap_nodes(graph, zergs, y);
					} else {
						// Too many removals
						destroy_graph(xless);
						destroy_graph(yless);
						destroy_graph(graph);
						return NULL;
					}
				} else {
					get_nodes_to_kill(xless, NULL);
					get_nodes_to_kill(yless, NULL);
				}

				// Finds which node required more removals
				if ((xless->v - yless->v) >= 0) {
					del_node(graph, y, zergs);
					destroy_graph(xless);
					destroy_graph(yless);

					break;
				}

				del_node(graph, x, zergs);
				destroy_graph(xless);
				destroy_graph(yless);
			}
		}
	}

	// Checks if too many deletions
	if ((graph->start_v - graph->v) > graph->del_limit) {
		destroy_graph(graph);
		return NULL;
	}

	return graph;
}

void set_zero(int *array, int size)
{
	/*
	 * Zeros out array
	 */

	for (int i = 0; i < size; i++)
		array[i] = 0;
}
