#pragma once

#include <stdint.h>

struct Company {
	char *symbol;
	char *name;
	long dollar;
};

struct Stock {
	struct Stock *parent;
	struct Company *company;
	struct Stock *left;
	struct Stock *right;
};

FILE *open_file(char *file);

struct Stock *parse_seed(FILE *seed, uint64_t *size);

void destroy(struct Stock *node);

void clear_buffer(char *buff, int len);

int get_symbol(char *line, char *sym);

int get_name(char *line, char *name);

char *double_from_str(char *string_ptr, double *number);

struct Stock *make_node(char *symbol, char *name, int dollar);

struct Stock *create_root(FILE *seed, uint64_t *size);

void print_node(struct Stock *node);

struct Stock *insert(struct Stock *tree, struct Stock *new, uint64_t *size_ptr);

void post_order(struct Stock *root, void (*fun)(struct Stock *));

struct Stock *parse_updates(struct Stock *root, uint64_t *size_ptr);

struct Stock *search(struct Stock *root, char *symbol);

int update_node(struct Stock *node, int dollar, char *name, char sign);

void quicksort(struct Stock **arr, uint64_t l, uint64_t r);

int partition(struct Stock **arr, uint64_t l, uint64_t r);

void populate_array(struct Stock *root, struct Stock **arr);

void sort_print(struct Stock *root, uint64_t *size_ptr);

void add_to_array(struct Stock **arr, struct Stock *node);