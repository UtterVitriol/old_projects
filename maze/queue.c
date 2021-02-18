#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "queue.h"

Queue *queue_create(int size)
{
	/*
	 * allocs queue
	 */

	Queue *new = malloc(sizeof(Queue));

	new->queue = malloc(size * sizeof(int));

	new->head = 0;
	new->tail = 0;
	new->len = size;
	new->size = 0;

	return new;
}

void queue_destroy(Queue *queue)
{
	/*
	 * frees queue
	 */

	free(queue->queue);
	free(queue);
}

Node *queue_front(Queue *queue)
{
	/*
	 * extracts first item in queue
	 */

	if (queue_size(queue) > 0) {
		return queue->queue[queue->head];
	}

	puts("empty queue");
	return NULL;
}

void queue_enqueue(Queue *queue, Node *val)
{
	/*
	 * inserts item into queue
	 */

	if (queue->head == queue->tail && queue->size > 0) {
		puts("queue full");
		return;
	}

	queue->queue[queue->tail] = val;
	queue->tail++;

	queue->size++;

	if (queue->tail == queue->len) {
		queue->tail = 0;
	}
}

void queue_dequeue(Queue *queue)
{
	/*
	 * removes first item from queue
	 */

	if (queue->size > 0) {
		queue->size--;
		queue->head++;
	} else {
		puts("queue empty");
		return;
	}

	if (queue->head == queue->len) {
		if (queue->tail > 0) {
			queue->head = 0;
			queue->size--;
			return;
		}
	}
}

int queue_size(Queue *queue)
{
	/*
	 * returns number of items in queue
	 */

	return queue->size;
}

int queue_empty(Queue *queue)
{
	/*
	 * checks if queue is empty
	 */

	if (queue->head == queue->tail || queue->size <= 0) {
		return 1;
	} else {
		return 0;
	}
}
