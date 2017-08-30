// RUN: %clang_cc1 -fcilkplus -Wmaybe-uninitialized -fsyntax-only -verify %s
// RUN: %clang_cc1 -fcilkplus -Wsometimes-uninitialized -Wconditional-uninitialized -fsyntax-only -verify %s



int foo(int a) {
  int x; // expected-note {{initialize the variable 'x' to silence this warning}}
  switch (a) {
  case 1: x = 1; break;
  case 2: x = 2; break;
  }
  return x; // expected-warning {{variable 'x' may be uninitialized when used here}}
}

int foo2(int a) {
  int x;
  _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
  switch (a) {
  case 1: x = 1; break;
  case 2: x = 2; break;
  }
  return x;
}
