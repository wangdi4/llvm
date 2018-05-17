// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaNoVector -fsyntax-only -verify %s
// expected warning

void foo(int lb, int ub, int *a, int *b) {
#pragma novector (enable) // expected-warning {{extra tokens at end of '#pragma novector' - ignored}}
  for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
}
