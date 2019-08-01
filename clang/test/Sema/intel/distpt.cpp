// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify

void foo() {
  int i;
  int a[10], b[10];
  #pragma distribute_point
  i = 0; // expected-warning {{statement following '#pragma distribute_point' must be a loop or within a loop}}
  for (i = 0; i < 10; ++i) {
    #pragma distribute_point
    a[i] = b[i] = 0;
  }
  for (i = 0; i < 10; ++i)
    #pragma distribute_point
    a[i] = b[i] = 5;
  #pragma distribute_point
  a[i] = b[i] = 5; // expected-warning {{statement following '#pragma distribute_point' must be a loop or within a loop}}
}
