/*
 *usage: ./intersect [-lui] [filename] [filename2] [filename3] .....
 *"-" can be used for the first filename and input will be taken from stdin-
 *instead
 *
 *l - sorts words by length u - sorts words in ascii order i - ignore
 *leading/trailing punctuation for matching
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "intersect.h"

int main(int argc, char **argv)
{
	handle_args(argc, argv);
}

void handle_args(int argc, char **argv)
{
	/*
	 * handles arguments, calls file i/o, sorting and printing functions
	 */

	int opt;
	int opt_count = 0;

	char opts[10] = {'\0'};

	while ((opt = getopt(argc, argv, "lui")) != -1) {

		// checks for duplicate flags
		if (strchr(opts, opt)) {
			print_help();
			return;
		}

		switch (opt) {
		case 'l':
			opts[opt_count] = opt;
			opt_count++;
			break;
		case 'u':
			opts[opt_count] = opt;
			opt_count++;
			break;
		case 'i':
			opts[opt_count] = opt;
			opt_count++;
			break;
		case '?':
			print_help();
			return;
		}
	}

	// print help if no files follow args
	if ((argc - optind) < 2) {
		print_help();
		return;
	}

	// get a number corresponding to which flags were present
	int options = 0;

	for (size_t i = 0; i < strlen(opts); i++) {
		if (opts[i] == 'l') {
			options += 1;
		} else if (opts[i] == 'u') {
			options += 3;
		}
	}

	int i = optind;

	int i_flag = 0;

	if (strchr(opts, 'i')) {
		i_flag = 1;
	}

	FILE *fp = open_file(argv[i]);

	Table *table = NULL;

	// read inition words from first file or stdin
	if (i_flag) {
		table = read_first_punct(fp, str_no_punct_cmp, nopunct_hash);
	} else {
		table = read_first(fp, str_case_cmp, hash);
	}

	i++;

	int file_num = 0;

	// loop through files
	if (table) {
		file_num++;
		for (; i < argc; i++) {
			if (i_flag) {
				compare_file_punct(table, argv[i], &file_num,
						   str_no_punct_cmp,
						   nopunct_hash);
			} else {
				compare_file(table, argv[i], &file_num,
					     str_case_cmp, hash);
			}
		}
	} else {
		return;
	}

	sort_print(table, file_num, options);
	destroy_table(table);
}

bool compare_file_punct(Table *table, char *f_name, int *file_num,
			int (*comp)(char *, char *), u64 (*hash)(char *))
{
	/*
	 * Reads words from file and compares them to initial file
	 * Ignores leading/trailing punctuation
	 */

	FILE *fp = fopen(f_name, "r");

	if (!fp) {
		fprintf(stderr, "File: \"%s\", does not exist...\n", f_name);
		return false;
	}

	*file_num = *file_num + 1;

	char *word = NULL;
	char *line = NULL;
	size_t size = 0;

	int all_punct_flag = 1;

	// loop line by line
	while (getline(&line, &size, fp) > 0) {

		int length = strlen(line);

		if (line[length - 1] == '\n') {
			line[length - 1] = '\0';
		}

		word = strtok(line, " \t");

		if (word) {

			// check if word is all punctuation
			for (size_t i = 0; i < strlen(word); i++) {
				if (!ispunct(word[i])) {
					all_punct_flag = 0;
					break;
				}
			}

			if (!all_punct_flag) {
				all_punct_flag = 1;
				search_table(table, word, file_num, comp, hash);
			}
		}

		word = strtok(NULL, " \t");

		while (word) {

			// check if word is all punctuation
			for (size_t i = 0; i < strlen(word); i++) {
				if (!ispunct(word[i])) {
					all_punct_flag = 0;
					break;
				}
			}

			if (!all_punct_flag) {
				all_punct_flag = 1;
				search_table(table, word, file_num, comp, hash);
			}
			word = strtok(NULL, " \t");
		}
	}

	if (line) {
		free(line);
	}
	fclose(fp);

	return true;
}

Table *read_first_punct(FILE *fp, int (*comp)(char *, char *),
			u64 (*hash)(char *))
{
	/*
	 * Read words from initial file and generate hash table
	 * Ignores leading/trailing punctuation
	 */

	char *word = NULL;
	char *line = NULL;
	size_t size = 0;

	Table *table = NULL;

	table = init_table();

	if (!table) {
		fprintf(stderr, "Out of Memory...\n");
		return NULL;
	}

	int all_punct_flag = 1;

	// loop line by line
	while (getline(&line, &size, fp) > 0) {

		int length = strlen(line);

		if (line[length - 1] == '\n') {
			line[length - 1] = '\0';
		}

		word = strtok(line, " \t");

		if (word) {
			if (asprintf(&word, "%s", word) < 1) {
				continue;
			}

			// check if word is all punctuation
			for (size_t i = 0; i < strlen(word); i++) {
				if (!ispunct(word[i])) {
					all_punct_flag = 0;
					break;
				}
			}

			if (all_punct_flag) {
				if (word) {
					free(word);
				}
			} else {
				all_punct_flag = 1;

				if (!insert_table(table, word, comp, hash)) {
					fprintf(stderr, "Out of Memory...\n");
					destroy_table(table);
					if (line) {
						free(line);
					}
					fclose(fp);
					return NULL;
				}
			}
		}

		word = strtok(NULL, " \t");

		while (word) {
			if (asprintf(&word, "%s", word) <= 0) {
				break;
			}

			// check if word is all punctuation
			for (size_t i = 0; i < strlen(word); i++) {
				if (!ispunct(word[i])) {
					all_punct_flag = 0;
					break;
				}
			}

			if (all_punct_flag) {
				if (word) {
					free(word);
				}
			} else {
				all_punct_flag = 1;
				if (!insert_table(table, word, comp, hash)) {
					fprintf(stderr, "Out of Memory...\n");
					destroy_table(table);
					if (line) {
						free(line);
					}
					fclose(fp);
					return NULL;
				}
			}

			word = strtok(NULL, " \t");
		}
	}

	if (line) {
		free(line);
	}
	fclose(fp);

	// check if any words word added to table
	if (table->count > 0) {
		return table;
	}

	destroy_table(table);
	fprintf(stderr, "No words found...\n");
	return NULL;
}

bool compare_file(Table *table, char *f_name, int *file_num,
		  int (*comp)(char *, char *), u64 (*hash)(char *))
{
	/*
	 * Reads words from file and compares them to initial file
	 */

	FILE *fp = fopen(f_name, "r");

	if (!fp) {
		fprintf(stderr, "File: \"%s\", does not exist...\n", f_name);
		return false;
	}

	*file_num = *file_num + 1;

	char *word = NULL;
	char *line = NULL;
	size_t size = 0;

	// loop line by line
	while (getline(&line, &size, fp) > 0) {

		int length = strlen(line);

		if (line[length - 1] == '\n') {
			line[length - 1] = '\0';
		}

		word = strtok(line, " \t");

		if (word) {
			search_table(table, word, file_num, comp, hash);
		} else {
			continue;
		}

		word = strtok(NULL, " \t");

		while (word) {
			search_table(table, word, file_num, comp, hash);
			word = strtok(NULL, " \t");
		}
	}

	if (line) {
		free(line);
	}

	fclose(fp);

	return true;
}

Table *read_first(FILE *fp, int (*comp)(char *, char *), u64 (*hash)(char *))
{
	/*
	 * Read words from initial file and generate hash table
	 */

	char *word = NULL;
	char *line = NULL;
	size_t size = 0;

	Table *table = init_table();

	if (!table) {
		fprintf(stderr, "Out of Memory...\n");
		return NULL;
	}

	// loop line by line
	while (getline(&line, &size, fp) > 0) {

		int length = strlen(line);

		if (line[length - 1] == '\n') {
			line[length - 1] = '\0';
		}

		word = strtok(line, " \t");

		if (word) {
			if (asprintf(&word, "%s", word) < 1) {
				continue;
			}

			if (!insert_table(table, word, comp, hash)) {
				fprintf(stderr, "Out of Memory...\n");
				destroy_table(table);
				if (line) {
					free(line);
				}
				fclose(fp);
				return NULL;
			}

		} else {
			continue;
		}

		word = strtok(NULL, " \t");

		while (word) {
			if (asprintf(&word, "%s", word) <= 0) {
				break;
			}

			if (!insert_table(table, word, comp, hash)) {
				fprintf(stderr, "Out of Memory...\n");
				destroy_table(table);
				if (line) {
					free(line);
				}
				fclose(fp);
				return NULL;
			}

			word = strtok(NULL, " \t");
		}
	}

	if (line) {
		free(line);
	}
	fclose(fp);

	// check if any words were added to table
	if (table->count > 0) {
		return table;
	}

	destroy_table(table);
	fprintf(stderr, "No words found...\n");
	return NULL;
}

FILE *open_file(char *f_name)
{
	/*
	 * Used to get stdin if first filename is "-"
	 */

	if (!strcmp(f_name, "-")) {
		return stdin;
	}

	FILE *fp = fopen(f_name, "r");

	if (!fp) {
		fprintf(stderr, "File: \"%s\", does not exist...\n\n", f_name);
		exit(1);
	}

	return fp;
}

void sort_print(Table *table, int file_num, int options)
{
	/*
	 * Counts common words, generates array of words, sorts and prints them
	 */

	int count = 0;
	char **words = NULL;

	// count common words
	for (int i = 0; i < BUCKETS; i++) {
		if (table->words[i]) {
			Word *temp = table->words[i];

			while (temp) {
				if (temp->count == file_num) {
					count++;
				}
				temp = temp->next;
			}
		}
	}

	if (count < 1)
		return;

	u32 index = 0;

	// alloc array
	words = malloc(count * sizeof(char *));

	// populate array
	for (int i = 0; i < BUCKETS; i++) {
		if (table->words[i]) {
			Word *temp = table->words[i];

			while (temp) {
				if (temp->count == file_num) {
					words[index] = temp->value;
					index++;
				}
				temp = temp->next;
			}
		}
	}

	/*
	 * Call quick sort with function depending on flags
	 * 3 = '-u'
	 * 1 = '-l'
	 * 4 = '-lu'
	 */
	if (options == 3) {
		quicksort(words, 0, index - 1, str_cmp);
	} else if (options == 1) {
		quicksort(words, 0, index - 1, strlen_a);
	} else if (options == 4) {
		quicksort(words, 0, index - 1, strlen_l);

	} else {
		quicksort(words, 0, index - 1, str_case_cmp);
	}

	// print words
	for (u32 i = 0; i < index; i++) {
		printf("%s\n", words[i]);
	}

	free(words);
}

int partition(char **arr, int l, int r, int (*comp)(char *, char *))
{
	/*
	 * Hoare's partition
	 */
	char *piv = arr[l];
	char *temp = NULL;

	l = l - 1;
	r = r + 1;
	while (true) {
		do {
			l++;
		} while (comp(arr[l], piv) < 0);

		do {
			r--;
		} while (comp(arr[r], piv) > 0);

		if (l < r) {
			temp = arr[l];
			arr[l] = arr[r];
			arr[r] = temp;
		} else {
			return r;
		}
	}
}

void quicksort(char **arr, int l, int r, int (*comp)(char *, char *))
{
	/*
	 * Quickly sorts array
	 */
	int p = 0;

	if (l < r) {
		p = partition(arr, l, r, comp);
		quicksort(arr, l, p, comp);
		quicksort(arr, p + 1, r, comp);
	}
}

Table *init_table(void)
{
	/*
	 * Initialize hash table
	 */

	Table *new = NULL;

	new = malloc(sizeof(*new));

	if (!new) {
		return NULL;
	}

	new->words = calloc(BUCKETS, sizeof(Word *));

	if (!new->words) {
		return NULL;
	}
	new->size = BUCKETS;
	new->count = 0;

	return new;
}

void destroy_table(Table *table)
{
	/*
	 * free table
	 */

	Word *temp = NULL;
	Word *last = NULL;

	for (int i = 0; i < BUCKETS; i++)
		if (table->words[i]) {
			temp = table->words[i];

			while (temp->next) {
				temp = temp->next;
			}
			while (temp) {
				last = temp->prev;
				free(temp->value);
				free(temp);
				temp = last;
			}
		}

	free(table->words);
	free(table);
}

bool insert_table(Table *table, char *word, int (*comp)(char *, char *),
		  u64 (*hash)(char *))
{
	/*
	 * Insert word into table if word does not already exist
	 */

	Word *new = NULL;

	new = calloc(1, sizeof(Word));

	if (!new) {
		fprintf(stderr, "Out of Memory...\n");
		return false;
	}
	new->value = word;

	new->key = hash(new->value);

	new->next = NULL;
	new->prev = NULL;
	new->count = 1;

	int pos = new->key % BUCKETS;

	Word *temp = NULL;

	if (!table->words[pos]) {
		table->words[pos] = new;
		table->count++;
		return true;
	}
	temp = table->words[pos];

	while (temp->next) {
		if (new->key == temp->key) {
			if (comp(new->value, temp->value) == 0) {
				free(new->value);
				free(new);
				return true;
			}
		}
		temp = temp->next;
	}

	if (comp(new->value, temp->value) == 0) {
		free(new->value);
		free(new);
		return true;
	}

	temp->next = new;
	new->prev = temp;
	table->count++;
	return true;
}

bool search_table(Table *table, char *word, int *file_num,
		  int (*comp)(char *, char *), u64 (*hash)(char *))
{
	/*
	 * Search table for word
	 */

	u64 key = hash(word);
	int pos = key % BUCKETS;

	Word *temp = table->words[pos];

	if (temp) {
		while (temp->next) {
			if (key == temp->key) {
				if (comp(word, temp->value) == 0) {
					if (temp->count == (*file_num - 1)) {
						temp->count++;
						return true;
					}
				}
			}
			temp = temp->next;
		}

		if (key == temp->key) {
			if (comp(word, temp->value) == 0) {
				if (temp->count == (*file_num - 1)) {
					temp->count++;
					return true;
				}
			}
		}
	}

	return false;
}

int str_case_cmp(char *word1, char *word2)
{
	return strcasecmp(word1, word2);
}

int str_cmp(char *word1, char *word2)
{
	return strcmp(word1, word2);
}

int strlen_a(char *word1, char *word2)
{
	/*
	 * Alphabetical string length compare
	 */

	int result1 = strlen(word1);
	int result2 = strlen(word2);

	if (result1 == result2) {
		return strcasecmp(word1, word2);
	}

	return result1 - result2;
}

int strlen_l(char *word1, char *word2)
{
	/*
	 * Lexicographical string length compare
	 */

	int result1 = strlen(word1);
	int result2 = strlen(word2);

	if (result1 == result2) {
		return strcmp(word1, word2);
	}

	return result1 - result2;
}

u64 hash(char *str)
{
	/*
	 * Hashes string ignoring case
	 */
	// This based on djb2 hash
	// Name: Dan Bernstein
	// Location: http://www.cse.yorku.ca/~oz/hash.html

	unsigned long hash = 5381;
	int c;

	while ((c = tolower(*str++))) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

u64 nopunct_hash(char *str)
{
	/*
	 * Hashes string ingoring case and punctuation
	 */
	// This based on djb2 hash
	// Name: Dan Bernstein
	// Location: http://www.cse.yorku.ca/~oz/hash.html

	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) {
		if (!ispunct(c)) {
			c = tolower(c);
			hash = ((hash << 5) + hash) + c;
		}
	}
	return hash;
}

int str_no_punct_cmp(char *word1, char *word2)
{
	/*
	 * Alphabetic string comparison
	 * Ignores leading/trailing punctuation
	 */

	char *temp1 = word1;
	char *temp2 = word2;

	int result = 0;

	// move past leading punctuation
	for (size_t i = 0; i < strlen(word1); i++) {
		if (isalpha(word1[i]) != 0) {
			break;
		}
		temp1++;
	}

	// move past leading punctuation
	for (size_t i = 0; i < strlen(word2); i++) {
		if (isalpha(word2[i]) != 0) {
			break;
		}
		temp2++;
	}

	char c1 = 'a';
	char c2 = 'a';

	/*
	 * walk words comparing characters
	 * break on trailing punctuation or end of word
	 */
	while (c1 && c2) {
		c1 = tolower(*temp1);
		c2 = tolower(*temp2);
		if (ispunct(c1) || ispunct(c2)) {
			break;
		}

		temp1++;
		temp2++;
		result += c1 - c2;
	}

	return result;
}

void print_help(void)
{
	/*
	 * help message
	 */

	fprintf(stderr, "usage: ./intersect ");
	fprintf(stderr, "[-lui] [filename] [filename2] [filename3] .....\n");
	fprintf(stderr,
		"\" - \" can be used for the first filename and input ");
	fprintf(stderr, "will be taken from stdin instead\n");
	fprintf(stderr, "l - sorts words by length\n");
	fprintf(stderr, "u - sorts words in ascii order\n");
	fprintf(stderr, "i - ignore leading/trailing ");
	fprintf(stderr, "punctuation for matching\n");
}
