#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "ticker.h"

int main(int argc, char **argv)
{
	// driver function for ticker

	if (argc != 2) {
		puts("Expected seed file as argument...");
		return 1;
	}

	FILE *seed = open_file(argv[1]);

	if (!seed)
		return 1;

	uint64_t *size_ptr = malloc(sizeof(unsigned long long));
	*size_ptr = 0;

	puts("Reading seed file...");
	struct Stock *root = parse_seed(seed, size_ptr);

	puts("\nReady for updates...");
	root = parse_updates(root, size_ptr);

	puts("\nSorting stocks...");
	sort_print(root, size_ptr);

	post_order(root, destroy);
	free(size_ptr);
	fclose(seed);
}

void sort_print(struct Stock *root, uint64_t *size_ptr)
{
	// creats array, populates it, sorts it and prints it

	uint64_t size = *size_ptr;

	struct Stock **array = malloc(*size_ptr * sizeof(struct Stock *));

	populate_array(root, array);

	quicksort(array, 0, size - 1);

	for (uint64_t i = 0; i < size; i++)
		print_node(array[i]);

	free(array);
}

void populate_array(struct Stock *root, struct Stock **arr)
{
	// loop to add nodes to array

	if (root == NULL)
		return;

	populate_array(root->left, arr);
	add_to_array(arr, root);
	populate_array(root->right, arr);
}

void add_to_array(struct Stock **arr, struct Stock *node)
{
	// adds node to array

	static uint64_t index = 0;

	arr[index] = node;
	index++;
}

int partition(struct Stock **arr, uint64_t l, uint64_t r)
{
	// partitions array for sorting

	long piv = arr[l]->company->dollar;

	l = l - 1;
	r = r + 1;

	struct Stock *temp = NULL;

	while (1) {
		do {
			l++;
		} while (arr[l]->company->dollar < piv);

		do {
			r--;
		} while (arr[r]->company->dollar > piv);

		if (l < r) {
			temp = arr[l];
			arr[l] = arr[r];
			arr[r] = temp;
		} else {
			return r;
		}
	}

	return l;
}

void quicksort(struct Stock **arr, uint64_t l, uint64_t r)
{
	// sorts array

	uint64_t p = 0;

	if (l < r) {
		p = partition(arr, l, r);
		quicksort(arr, l, p);
		quicksort(arr, p + 1, r);
	}
}

struct Stock *parse_updates(struct Stock *root, uint64_t *size_ptr)
{
	// parses updates from stdin

	struct Stock *node = NULL;

	size_t size = 0;

	char *line = NULL;
	char *val_ptr = NULL;

	char symbol[6] = {'\0'};
	char name[64] = {'\0'};
	char temp[1] = {'\0'};

	int result = 0;
	double val = 0;

	int dollar = 0;

	char sign = '\0';

	while (1) {
		if ((getline(&line, &size, stdin)) <= 0)
			break;

		result = get_symbol(line, symbol);

		if (result < 0)
			continue;

		val_ptr = line + result;

		if (val_ptr[0] != '+' && val_ptr[0] != '-') {
			fprintf(stderr, "Bad update, no sign\n");
			continue;
		} else {
			sign = val_ptr[0];
			val_ptr++;
		}

		val_ptr = double_from_str(val_ptr, &val);

		if (!val_ptr || val > 100000) {
			fprintf(stderr, "Bad stock value\n");
			continue;
		}

		dollar = (val * 100) + .5;

		if (val_ptr[0] == EOF || feof(stdin)) {
			node = search(root, symbol);
			if (!node) {
				if (sign == '-') {
					fprintf(stderr,
						"Can't add't negative ");
					fprintf(stderr, "value stock...\n");
					continue;
				}
				if (val >= 1000000) {
					fprintf(stderr,
						"Can't add stock with value ");
					fprintf(stderr,
						"greater than $1000000...\n");
					continue;
				}
				node = make_node(symbol, temp, dollar);
				insert(root, node, size_ptr);
			} else {
				if (update_node(node, dollar, temp, sign))
					continue;
			}
			break;
		}

		if (val_ptr[0] == '\n') {
			node = search(root, symbol);
			if (!node) {
				if (sign == '-') {
					fprintf(stderr, "Can't add negative ");
					fprintf(stderr, "value stock...\n");
					continue;
				}
				if (val > 100000) {
					fprintf(stderr,
						"Can't add stock with value ");
					fprintf(stderr,
						"greater than $1000000...\n");
					continue;
				}
				node = make_node(symbol, temp, dollar);
				insert(root, node, size_ptr);
			} else {
				if (update_node(node, dollar, temp, sign))
					continue;
			}
			continue;
		}

		if (val_ptr[0] != 32) {
			fprintf(stderr, "Expected space after value...\n");
			continue;
		}

		val_ptr++;

		result = get_name(val_ptr, name);

		if (result)
			continue;

		node = search(root, symbol);

		if (!node) {
			if (sign == '-') {
				fprintf(stderr, "Can't add negative ");
				fprintf(stderr, "value stock...\n");
				continue;
			}
			if (val > 100000) {
				fprintf(stderr, "Can't add stock with value ");
				fprintf(stderr, "greater than $1000000...\n");
				continue;
			}
			node = make_node(symbol, name, dollar);
			insert(root, node, size_ptr);
		} else {
			if (update_node(node, dollar, name, sign))
				continue;
		}
	}

	if (line)
		free(line);

	return root;
}

int update_node(struct Stock *node, int dollar, char *name, char sign)
{
	// updates node with new information

	double current = 0;
	double new = 0;

	current = (double)node->company->dollar / 100;
	new = (double)dollar / 100;

	switch (sign) {
	case '+':
		if ((current + new) > 1000000) {
			fprintf(stderr, "Update can not make value go ");
			fprintf(stderr, "above $100000...\n");
			return 1;
		}
		node->company->dollar += dollar;
		break;
	case '-':
		if ((current - new) < 0.01) {
			fprintf(stderr, "Update can not make value go ");
			fprintf(stderr, "below $.01...\n");
			return 1;
		}
		node->company->dollar -= dollar;
		break;
	}

	if (strlen(name) > 0) {
		if (strcmp(node->company->name, name) == 0)
			return 0;

		free(node->company->name);
		node->company->name = calloc(strlen(name) + 1, sizeof(char));
		strcpy(node->company->name, name);
	}
	return 0;
}

FILE *open_file(char *file)
{
	// get file pointer to seed file

	FILE *fp;

	fp = fopen(file, "r");

	if (!fp) {
		printf("File: \"%s\", does not exist...\n\n", file);
		return NULL;
	}

	return fp;
}

struct Stock *parse_seed(FILE *seed, uint64_t *size_ptr)
{
	// parses stdin for stocks

	struct Stock *root = create_root(seed, size_ptr);

	if (!root) {
		fprintf(stderr, "No good input in seed file...\n");
		free(size_ptr);
		fclose(seed);
		exit(1);
	}

	struct Stock *node = NULL;

	size_t size = 0;

	char *line = NULL;
	char *val_ptr = NULL;

	char symbol[6] = {'\0'};
	char name[64] = {'\0'};

	int result = 0;
	double val = 0;

	int dollar = 0;

	while (1) {
		if ((getline(&line, &size, seed)) <= 0)
			break;

		result = get_symbol(line, symbol);

		if (result < 0)
			continue;

		val_ptr = line + result;

		val_ptr = double_from_str(val_ptr, &val);

		if (!val_ptr || val > 100000) {
			fprintf(stderr, "Bad stock value\n");
			continue;
		}

		dollar = (val * 100) + .5;

		if (strlen(val_ptr) < 1) {
			if (feof(seed)) {
				node = search(root, symbol);
				if (node) {
					fprintf(stderr, "Repeated symbol...\n");
					continue;
				}
				clear_buffer(name, 64);
				node = make_node(symbol, name, dollar);
				insert(root, node, size_ptr);
				break;
			}
		}

		if (val_ptr[0] == '\n' || val_ptr[0] == '#') {
			node = search(root, symbol);
			if (node) {
				fprintf(stderr, "Repeated symbol...\n");
				continue;
			}
			clear_buffer(name, 64);
			node = make_node(symbol, name, dollar);
			insert(root, node, size_ptr);
			continue;
		}

		if (val_ptr[0] != 32) {
			puts("expected space after value");
			continue;
		}

		val_ptr++;

		result = get_name(val_ptr, name);

		if (result)
			continue;

		node = search(root, symbol);
		if (node) {
			fprintf(stderr, "Repeated symbol...\n");
			continue;
		}
		node = make_node(symbol, name, dollar);
		insert(root, node, size_ptr);
	}

	if (line)
		free(line);

	return root;
}

struct Stock *create_root(FILE *seed, uint64_t *size_ptr)
{
	// parses stdin for first valid stock

	struct Stock *root = NULL;

	size_t size = 0;

	char *line = NULL;
	char *val_ptr = NULL;

	char symbol[6] = {'\0'};
	char name[64] = {'\0'};
	char temp[1] = {'\0'};

	int result = 0;
	double val = 0;

	int dollar = 0;

	while (1) {
		if ((getline(&line, &size, seed)) <= 0)
			break;

		result = get_symbol(line, symbol);

		if (result < 0)
			continue;

		val_ptr = line + result;

		val_ptr = double_from_str(val_ptr, &val);

		if (!val_ptr || val > 1000000) {
			fprintf(stderr, "Bad stock value\n");
			continue;
		}

		dollar = (val * 100) + .05;

		if (strlen(val_ptr) < 1) {

			if (feof(seed)) {
				root = make_node(symbol, temp, dollar);

				break;
			}
		}

		if (val_ptr[0] == '\n' || val_ptr[0] == '#') {
			root = make_node(symbol, temp, dollar);
			break;
		}

		if (val_ptr[0] != 32) {
			root = make_node(symbol, temp, dollar);
			break;
		}

		val_ptr++;

		result = get_name(val_ptr, name);

		if (result)
			continue;

		root = make_node(symbol, name, dollar);
		break;
	}

	if (line)
		free(line);

	*size_ptr = *size_ptr + 1;

	return root;
}

void print_node(struct Stock *node)
{
	// prints a node

	printf("%s ", node->company->symbol);
	printf("%.2f ", (double)node->company->dollar / 100);

	if (node->company->name)
		printf("%s\n", node->company->name);
	else
		puts("");
}

struct Company *company_create(char *symbol, char *name, int dollar)
{
	// allocs a new company

	struct Company *new = malloc(sizeof(struct Company));

	new->symbol = calloc(strlen(symbol) + 1, sizeof(char));
	strcpy(new->symbol, symbol);
	new->name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(new->name, name);
	new->dollar = dollar;

	return new;
}

struct Stock *stock_create(struct Company *company)
{
	// mallocs a new company

	struct Stock *new = malloc(sizeof(struct Stock));

	new->company = company;
	new->parent = NULL;
	new->left = NULL;
	new->right = NULL;

	return new;
}

struct Stock *make_node(char *symbol, char *name, int dollar)
{
	// creates new node

	struct Company *new_c = company_create(symbol, name, dollar);
	struct Stock *new_s = stock_create(new_c);

	return new_s;
}

int get_symbol(char *line, char *sym)
{
	// checks for and reads stock symbol

	int count = 0;

	clear_buffer(sym, 6);

	if (!isalpha(line[count])) {
		fprintf(stderr, "Bad symbol...\n");
		return -1;
	}

	while (1) {
		if (count > 5) {
			fprintf(stderr, "Bad symbol...\n");
			clear_buffer(sym, 6);
			return -1;
		}

		if (line[count] == ' ')
			return count + 1;

		if (!isalpha(line[count])) {
			fprintf(stderr, "Bad symbol...\n");
			return -1;
		}

		sym[count] = toupper(line[count]);
		count++;
	}

	puts("This is bad (get symbol)");
	return 0;
}

int get_name(char *line, char *name)
{
	// checks for and reads company name

	int count = 0;

	clear_buffer(name, 64);

	while (1) {
		if (count > 63) {
			fprintf(stderr, "Bad name length...\n");
			return 1;
		}

		if (line[count] == '\0' || line[count] == '\n' ||
		    line[count] == '#' || line[count] == EOF) {
			return 0;
		}

		name[count] = line[count];
		count++;
	}

	puts("This is bad (get name)");
	return 0;
}

void clear_buffer(char *buff, int len)
{
	// clears buffer

	for (int i = 0; i < len; i++)
		buff[i] = '\0';
}

char *double_from_str(char *string_ptr, double *number)
{
	// gets a double from a string

	char *position = string_ptr;

	if (isdigit(*position)) {
		*number = strtod(position, &position);
		return position;
	} else {
		return NULL;
	}

	return NULL;
}

struct Stock *insert_loop(struct Stock *tree, struct Stock *i)
{
	// loop for inserting a node into a BST
	if (strcmp(i->company->symbol, tree->company->symbol) == 0)
		return i;

	if (strcmp(i->company->symbol, tree->company->symbol) < 0) {
		if (tree->left == NULL) {
			tree->left = i;
			i->parent = tree;
		}

		return insert_loop(tree->left, i);
	}

	if (tree->right == NULL) {
		tree->right = i;
		i->parent = tree;
	}

	return insert_loop(tree->right, i);
}

struct Stock *insert(struct Stock *tree, struct Stock *new, uint64_t *size_ptr)
{
	// primer function to insert node into a BST

	new = insert_loop(tree, new);
	if (new->parent == NULL) {
		destroy(new);
		return NULL;
	}

	*size_ptr = *size_ptr + 1;
	return new;
}

void destroy(struct Stock *node)
{
	// frees a node

	free(node->company->symbol);
	free(node->company->name);
	free(node->company);
	free(node);
}

void post_order(struct Stock *root, void (*fun)(struct Stock *))
{
	// post order tree traversal, calling fun on each node
	if (root == NULL)
		return;

	post_order(root->left, fun);
	post_order(root->right, fun);

	fun(root);
}

struct Stock *search(struct Stock *root, char *symbol)
{
	// searches for a node, returns it

	if (root == NULL)
		return root;

	if (strcmp(symbol, root->company->symbol) == 0)
		return root;

	if (strcmp(symbol, root->company->symbol) < 0)
		return search(root->left, symbol);

	return search(root->right, symbol);
}
