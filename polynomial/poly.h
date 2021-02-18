#pragma once

#include <stdbool.h>

/**
 * \file poly.h
 * \brief Create and manipulate linked-lists containing polynomials.
 *
 * Requires linking the math library with "-lm".
 */

/**
 * \def term
 * \brief Polynomial node structure.
 */
struct term {
	int coeff;
	unsigned int exp;
	struct term *next;
};

/**
 * \def Polynomial
 * \brief A type definition for term.
 */
typedef struct term Polynomial;

/**
 * \fn struct term *Poly_create_term(int coeff, unsigned int exp)
 * \brief Creates polynomial node with coefficient and exponent
 * returns node.
 *
 * \param coeff The coefficient of the polynomial term.
 * \param exp The exponent to of the polynomial term .
 */
struct term *Poly_create_term(int coeff, unsigned int exp);

/**
 * \fn void Poly_destroy(Polynomial *p)
 * \brief Frees memory used by a polynomial.
 *
 * \param p The polynomial to destroy.
 */
void Poly_destroy(Polynomial *p);

/**
 * \fn void Poly_print(const Polynomial *p)
 * \brief Prints a polynomial to stdout.
 *
 * \param p The polynomial to print.
 */
void Poly_print(const Polynomial *p);

/**
 * \fn char *Poly_to_string(const Polynomial *p)
 * \brief Returns a newly-malloced string that displays the given polynomial
 * with the parameter as x.
 *
 * \param p The polynomial to convert to a string.
 */
char *Poly_to_string(const Polynomial *p);

/**
 * \fn Polynomial *Poly_add(const Polynomial *a, const Polynomial *b)
 * \brief Returns a newly-malloced polynomial that is the sum of the two
 * arguments.
 *
 * \param a The first polynomial to add.
 * \param b The second polynomial to add.
 */
Polynomial *Poly_add(const Polynomial *a, const Polynomial *b);

/**
 * \fn Polynomial *Poly_sub(const Polynomial *a, const Polynomial *b)
 * \brief Returns a newly-malloced polynomial that is the difference of the two
 * arguments.
 *
 * \param a The first polynomial to subtract.
 * \param b The second polynomial to subtract.
 */
Polynomial *Poly_sub(const Polynomial *a, const Polynomial *b);

/**
 * \fn bool Poly_equal(const Polynomial *a, const Polynomial *b)
 * \brief Returns true if the two arguments have the same terms, false
 * otherwise.
 *
 * \param a The first polynomial to check.
 * \param b The second polynomial to check.
 */
bool Poly_equal(const Polynomial *a, const Polynomial *b);

/**
 * \fn double Poly_eval(const Polynomial *p, double x)
 * \brief Evaluates the polynomial by substituting x in the variable of the
 * polynomial.
 *
 * \param p The polynomial to evaluate.
 * \param x The number to substitute for the parameter in the polynomial.
 */
double Poly_eval(const Polynomial *p, double x);

/**
 * \fn void Poly_iterate(Polynomial *p, void (*transform)(struct term *))
 * \brief Calls the function transform on each term of the polynomial.
 *
 * \param p The polynomial to iterate.
 * \param transorm A function variable, to be called on each term of the
 * polynomial.
 */
void Poly_iterate(Polynomial *p, void (*transform)(struct term *));
