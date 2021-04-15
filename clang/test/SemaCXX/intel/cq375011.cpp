// Test semantics of the 'alloc_size' attribute.

// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

// Check that correctly-formed attribute produces no diagnostics.
void *f(int a) __attribute__((__alloc_size__(1)));
int *f1(int a, int b) __attribute__((__alloc_size__(1, 2)));
float *f2(double a, char *b, unsigned c, float d, int e)
    __attribute__((__alloc_size__(3, 5)));

// Check that attribute, applied to non-function, produces a warning.
int *v __attribute__((__alloc_size__(1))); // expected-warning{{'__alloc_size__' attribute only applies to non-K&R-style functions}}

// Check the amount of attribute's arguments.
int *f3(int a) __attribute__((__alloc_size__)); // expected-error{{'__alloc_size__' attribute takes at least 1 argument}}
int *f4(int a, int b) __attribute__((__alloc_size__(1, 2, 1))); // expected-error{{'__alloc_size__' attribute takes no more than 2 arguments}}

// Check that attribute's argument is an integer constant expression.
int *f5(int a) __attribute__((__alloc_size__(1.5))); // expected-error{{'__alloc_size__' attribute requires parameter 1 to be an integer constant}}

// Check that function's parameter specified by attribute's argument has an integer type.
int *f6(float a) __attribute__((__alloc_size__(1))); // expected-error{{'__alloc_size__' attribute argument may only refer to a function parameter of integer type}}
int *f7(int a, unsigned b, double c) __attribute__((__alloc_size__(1, 3))); // expected-error{{'__alloc_size__' attribute argument may only refer to a function parameter of integer type}}

// Check the bounds of attribute's arguments.
int *f8(int a) __attribute__((__alloc_size__(0))); // expected-error{{'__alloc_size__' attribute parameter 1 is out of bounds}}
int *f9(int a) __attribute__((__alloc_size__(2))); // expected-error{{'__alloc_size__' attribute parameter 1 is out of bounds}}

// Check that function declared with __alloc_size__ returns a pointer.
int f10(int a) __attribute__((__alloc_size__(1))); //expected-warning{{'__alloc_size__' attribute only applies to return values that are pointers}}

// Check struct members, declared with __alloc_size__ attribute.
struct X {
  int a;

  // Check implicit 'this' argument counts first.
  int *g1(int a) __attribute__((__alloc_size__(2))); // OK, 'this' is counted first, 'a' is the second parameter.
  int *g2(float a, int b, char *c, void *d, int e)
      __attribute__((__alloc_size__(3, 6)));         // OK, 'this' is counted first.
  int *g2(int a) __attribute__((__alloc_size__(1))); // expected-error{{'__alloc_size__' attribute is invalid for the implicit this argument}}
  int *g3(int a) __attribute__((__alloc_size__(3))); // expected-error{{'__alloc_size__' attribute parameter 1 is out of bounds}}
};
