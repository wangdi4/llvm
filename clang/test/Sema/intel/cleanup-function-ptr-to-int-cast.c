// CQ#371284
// RUN: %clang_cc1 -fsyntax-only -verify %s -DTEST0
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s -DTEST1

#ifdef TEST0

static void foo(int x) {}

static void test() {
  // Clang must generate an error here.
  int a __attribute__((cleanup(foo))); // expected-error {{'cleanup' function 'foo' parameter has type 'int' which is incompatible with type 'int *'}}
}

#elif TEST1

static void foo(int x) {} // expected-note {{'cleanup' function 'foo' has formal parameter of type 'int'}}

static void test() {
  int a __attribute__((cleanup(foo))); // expected-warning {{incompatible pointer to integer conversion sending 'int *' to parameter of type 'int'}}
}

#else

#error Unknown test mode

#endif


