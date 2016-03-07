// RUN: %clang_cc1 -fintel-compatibility -verify %s -DWARN
// RUN: %clang_cc1 -fintel-compatibility -fintel-ms-compatibility -verify %s

struct Empty { };
struct EmptyTest {struct Empty s; int i;};
#ifdef WARN
// expected-warning@+4{{initializer for aggregate with no elements requires}}
#else
// expected-error@+2{{initializer for aggregate with no elements requires}}
#endif
struct EmptyTest empty = { 3 };
