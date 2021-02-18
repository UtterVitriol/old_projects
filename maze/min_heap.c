// based on code from
// https://www.geeksforgeeks.org/dijkstras-algorithm-for-adjacency-list-representation-greedy-algo-8/

#include <stdio.h>
#include <stdlib.h>

#include "min_heap.h"

Heap *create_heap(int size)
{
	/*
	 * allocs heap
	 */

	Heap *new = malloc(sizeof(Heap));

	new->array = calloc(size, sizeof(Item *));
	new->count = 0;
	new->size = size;
	new->pos = malloc(size * sizeof(int));

	return new;
}

Item *create_item(int v, int dist)
{
	/*
	 * allocs new item for heap
	 */

	Item *new = malloc(sizeof(Item));

	new->v = v;
	new->dist = dist;

	return new;
}

void heap_insert(Heap *heap, int v, int dist)
{
	/*
	 * inserts item into heap
	 */

	Item *new = create_item(v, dist);

	heap->array[heap->count] = new;
	heap->count++;

	if (heap->count == 1) {
		return;
	}

	heapify_upshift(heap, heap->count - 1);
}

void heapify_upshift(Heap *heap, int index)
{
	/*
	 * heapifies after insert
	 */

	int p = (index - 1) / 2;

	if (heap->array[index]->dist < heap->array[p]->dist) {
		Item *temp = heap->array[index];

		heap->array[index] = heap->array[p];
		heap->array[p] = temp;
		heapify_upshift(heap, p);
	}
}

void heapify_downshift(int index, Heap *heap, int len)
{
	/*
	 * heapifies after extraction
	 */

	int left = (2 * index) + 1;
	int right = left + 1;
	int smallest = index;

	if (left < len &&
	    heap->array[left]->dist < heap->array[smallest]->dist) {
		smallest = left;
	}

	if (right < len &&
	    heap->array[right]->dist < heap->array[smallest]->dist) {
		smallest = right;
	}

	if (smallest != index) {
		Item *small = heap->array[smallest];
		Item *idx = heap->array[index];

		heap->pos[small->v] = index;
		heap->pos[idx->v] = smallest;

		Item *temp = heap->array[index];

		heap->array[index] = heap->array[smallest];
		heap->array[smallest] = temp;
		heapify_downshift(smallest, heap, len);
	}
}

Item *heap_extract(Heap *heap)
{
	/*
	 * extracts first element of heap
	 */

	if (heap->count == 0) {
		return NULL;
	}

	Item *root = heap->array[0];
	Item *last = heap->array[heap->count - 1];

	heap->array[0] = last;

	heap->pos[root->v] = heap->count - 1;
	heap->pos[last->v] = 0;
	heap->count--;

	heapify_downshift(0, heap, heap->count);

	return root;
}

void heap_destroy(Heap *heap)
{
	/*
	 * frees heap
	 */

	free(heap->array);
	free(heap->pos);
	free(heap);
}
