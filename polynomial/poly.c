#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "poly.h"

struct term *Poly_create_term(int coeff, unsigned int exp)
{
	/* Creates node with coefficient and exponent
	 * Returns node
	 */
	struct term *node = malloc(sizeof(*node));

	if (node) {
		node->coeff = coeff;
		node->exp = exp;
		node->next = NULL;
	}

	return node;
}

void Poly_destroy(Polynomial *eqn)
{
	// Frees each node in list

	while (eqn) {
		struct term *tmp = eqn->next;

		free(eqn);
		eqn = tmp;
	}
}

void Poly_print(const Polynomial *eqn)
{
	// prints each term in polynomial list

	if (!eqn)
		return;

	if (eqn->coeff) {
		printf("%c%d", eqn->coeff > 0 ? '+' : '\0', eqn->coeff);
		if (eqn->exp > 1)
			printf("x^%d", eqn->exp);
		else if (eqn->exp == 1)
			printf("x");

		printf(" ");
	}

	Poly_print(eqn->next);
}

void Poly_sort(Polynomial *poly)
{
	// sorts polynomial list in descending order based on exponent

	struct term *step = NULL;
	struct term *last = NULL;

	last = poly;
	step = poly->next;

	while (true) {
		while (true) {
			if (step == NULL)
				break;

			if (poly->exp < step->exp) {

				// swap exponents and coefficients

				poly->exp = poly->exp ^ step->exp;
				step->exp = poly->exp ^ step->exp;
				poly->exp = poly->exp ^ step->exp;

				poly->coeff = poly->coeff ^ step->coeff;
				step->coeff = poly->coeff ^ step->coeff;
				poly->coeff = poly->coeff ^ step->coeff;
			}

			if (poly->exp == step->exp) {

				// add terms, free leftover term

				poly->coeff += step->coeff;
				last->next = step->next;

				free(step);
				step = last;
			}

			last = step;
			step = step->next;
		}

		poly = poly->next;

		if (poly == NULL)
			return;

		step = poly->next;
	}
}

int num_digits(int val)
{
	// returns number of digits in integer

	return floor(log10(abs(val))) + 1;
}

char *Poly_to_string(const Polynomial *p)
{
	/* creates string containing polynomial
	 * returns calloc'ed string
	 */

	const Polynomial *temp = p;

	int size = 0;

	// get number of chars need to represent polynomial
	while (temp) {
		size += 5;
		size += num_digits(temp->coeff);
		size += num_digits(temp->exp);
		temp = temp->next;
	}

	char *eqn = NULL;

	// calloct string to store polynomial
	eqn = calloc(size, sizeof(char));

	if (!p)
		return NULL;

	// populate polynomial
	while (p != NULL) {
		if (p->coeff) {
			sprintf(eqn + strlen(eqn), "%c%d",
				p->coeff > 0 ? '+' : '-', abs(p->coeff));
			if (p->exp > 1)
				sprintf(eqn + strlen(eqn), "x^%d", p->exp);
			else if (p->exp == 1)
				sprintf(eqn + strlen(eqn), "x");

			sprintf(eqn + strlen(eqn), " ");
		}

		p = p->next;
	}

	return eqn;
}

Polynomial *Poly_add(const Polynomial *a, const Polynomial *b)
{
	/* Adds two polynomials
	 * returns result as new polynomial
	 */

	// create dummy node
	Polynomial *new = malloc(sizeof(Polynomial));
	Polynomial *step = NULL;

	step = new;

	while (a && b) {
		if (a->exp == b->exp) {

			// create node with sum of terms
			if (a->coeff + b->coeff != 0) {
				step->next = Poly_create_term(
				    a->coeff + b->coeff, a->exp);
				step = step->next;
			}
			a = a->next;
			b = b->next;
		} else if (a->exp > b->exp) {
			// create node with greater term
			step->next = Poly_create_term(a->coeff, a->exp);
			step = step->next;
			a = a->next;
		} else {
			// create node with greater term
			step->next = Poly_create_term(b->coeff, b->exp);
			step = step->next;
			b = b->next;
		}
	}

	// check for leftovers
	while (a) {
		// create leftover nodes
		step->next = Poly_create_term(a->coeff, a->exp);
		step = step->next;
		a = a->next;
	}

	// check for leftovers
	while (b) {
		// create leftover nodes
		step->next = Poly_create_term(b->coeff, b->exp);
		step = step->next;
		b = b->next;
	}

	step = new->next;

	// free dummy node
	free(new);
	new = step;
	return new;
}

Polynomial *Poly_sub(const Polynomial *a, const Polynomial *b)
{
	/* subtracs two polynomials
	 * returns result as new polynomial
	 */

	// create dummy node
	Polynomial *new = malloc(sizeof(Polynomial));
	Polynomial *step = NULL;

	step = new;

	while (a && b) {
		if (a->exp == b->exp) {
			// create node with difference of terms
			if (a->coeff - b->coeff != 0) {
				step->next = Poly_create_term(
				    a->coeff - b->coeff, a->exp);
				step = step->next;
			}
			a = a->next;
			b = b->next;
		} else if (a->exp > b->exp) {
			// create node with greater term
			step->next = Poly_create_term(a->coeff, a->exp);
			step = step->next;
			a = a->next;
		} else {
			// create node with greater term
			step->next = Poly_create_term(b->coeff, b->exp);
			step = step->next;
			b = b->next;
		}
	}

	// check for leftovers
	while (a) {
		// create leftover nodes
		step->next = Poly_create_term(a->coeff, a->exp);
		step = step->next;
		a = a->next;
	}

	// check for leftovers
	while (b) {
		// create leftover nodes
		step->next = Poly_create_term(b->coeff, b->exp);
		step = step->next;
		b = b->next;
	}

	step = new->next;

	// free dummy node
	free(new);
	new = step;
	return new;
}

bool Poly_equal(const Polynomial *a, const Polynomial *b)
{
	/* compares two polynomials
	 * returns true if they are equal, false if they are not
	 */

	while (a != NULL && b != NULL) {
		if (a->coeff == b->coeff && a->exp == b->exp) {
			a = a->next;
			b = b->next;
		} else {
			return false;
		}
	}

	return true;
}

double Poly_eval(const Polynomial *p, double x)
{
	/* evaluates a polynomial with, substituting the variable with x
	 * returns result of evalutation
	 */

	double result = 0;

	while (p != NULL) {
		result += (p->coeff * pow(x, p->exp));
		p = p->next;
	}

	return result;
}

void Poly_iterate(Polynomial *p, void (*transform)(struct term *))
{
	/* iterates all nodes in polynomial and calls-
	 * function pointed to by transform on each node
	 * sorts polynomial afterwards
	 */

	Polynomial *temp = p;

	while (temp != NULL) {
		transform(temp);
		temp = temp->next;
	}

	Poly_sort(p);
}
