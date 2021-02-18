#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BUCKETS 1000000
#define u32 uint32_t
#define u64 uint64_t

typedef struct Word {
	char *value;
	u64 key;
	int count;
	struct Word *next;
	struct Word *prev;

} Word;

typedef struct Table {
	Word **words;
	u32 size;
	u32 count;
} Table;

Table *init_table(void);

void destroy_table(Table *table);

bool insert_table(Table *table, char *word, int (*comp)(char *, char *),
		  u64 (*hash)(char *));
u64 hash(char *str);

Table *read_first(FILE *fp, int (*comp)(char *, char *), u64 (*hash)(char *));

bool compare_file(Table *table, char *f_name, int *file_num,
		  int (*comp)(char *, char *), u64 (*hash)(char *));

Table *read_first_punct(FILE *fp, int (*comp)(char *, char *),
			u64 (*hash)(char *));

bool compare_file_punct(Table *table, char *f_name, int *file_num,
			int (*comp)(char *, char *), u64 (*hash)(char *));

bool search_table(Table *table, char *word, int *file_num,
		  int (*comp)(char *, char *), u64 (*hash)(char *));

void sort_print(Table *table, int file_num, int options);

int partition(char **arr, int l, int r, int (*comp)(char *, char *));

int strlen_a(char *word1, char *word2);
int strlen_l(char *word1, char *word2);
int str_cmp(char *word1, char *word2);
int str_case_cmp(char *word1, char *word2);

void quicksort(char **arr, int l, int r, int (*comp)(char *, char *));
void handle_args(int argc, char **argv);
void print_help(void);
FILE *open_file(char *f_name);

int str_no_punct_cmp(char *word1, char *word2);
u64 nopunct_hash(char *str);