#pragma once

#include <stdbool.h>

#include "graph.h"

typedef struct Queue {
	int head;
	Node **queue;
	int tail;
	int len;
	int size;
} Queue;

Queue *queue_create(int size);
void queue_destroy(Queue *queue);
Node *queue_front(Queue *queue);
void queue_enqueue(Queue *queue, Node *val);
void queue_dequeue(Queue *queue);
int queue_size(Queue *queue);
int queue_empty(Queue *queue);
