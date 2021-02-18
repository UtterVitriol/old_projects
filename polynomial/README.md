# poly

poly is a simple c library for creating and manipulating simple polynomials

requires math library
## Installation

```bash
git clone git@git.umbc.tc:tdqc/tdqc8/jkaten/polynomial.git
```

## Usage

```c
#include "poly.h"

Polynomial *a = Poly_create_term(2, 2); // mallocs single term polynomial
Polynomial *b = Poly_create_term(2, 2); // mallocs single term polynomial

Poly_print(a); // prints polynomial to stdout

Polynomial *c = Poly_add(a, b); // mallocs polynomial with sum of polynomial a and b
Polynomial *c = Poly_sub(a, b); // mallocs polynomial with difference of polynomial a and b

Poly_equal(a, b); // returns true if polynomials are equal, false otherwise

Poly_eval(a, 2); // evaluates polynomial with 2 substituted for parameter

char *str = Poly_to_string(a); // returns polynomial as a malloc'd string

Poly_interate(a, fun); // Calls fun on each term of polynomial

Poly_destroy(a); // frees memory used by polynomial
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[donut](https://choosealicense.com/licenses/mit/)