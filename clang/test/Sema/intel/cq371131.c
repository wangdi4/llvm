// RUN: %clang_cc1 -verify %s

struct Empty { };
struct EmptyTest {struct Empty s; int i;};
struct EmptyTest empty = { 3 }; // expected-error{{initializer for aggregate with no elements requires}}
