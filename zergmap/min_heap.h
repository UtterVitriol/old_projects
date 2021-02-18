#pragma once

typedef struct Item {
	int v;
	int dist;
} Item;

typedef struct Heap {
	Item **array;
	int count;
	int size;
	int *pos;
} Heap;

Heap *create_heap(int size);
Item *create_item(int v, int dist);

void heapify_downshift(int index, Heap *heap, int len);
void heapify_upshift(Heap *heap, int index);
void heap_insert(Heap *heap, int v, int dist);
Item *heap_extract(Heap *heap);
void heap_destroy(Heap *heap);
