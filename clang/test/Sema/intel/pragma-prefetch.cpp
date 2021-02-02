// RUN: %clang_cc1 -verify -fintel-compatibility %s
//
// Test semantic warnings related to #pragma [no]prefetch
void prefetch() {
  int A;
  int B;

// expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma prefetch'}}
#pragma prefetch
   A = B;
// expected-error@+1 {{hint value, 0, must be between 1 and 4, inclusive}}
#pragma prefetch A:0
  for (int i; i < 10; ++i) {}
// expected-error@+1 {{hint value, 5, must be between 1 and 4, inclusive}}
#pragma prefetch A:5
  for (int i; i < 10; ++i) {}
// expected-error@+1 {{distance value must be greater than zero}}
#pragma prefetch A:4:0
  for (int i; i < 10; ++i) {}
  int p;
  int *pp = &p;
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch (pp + 1)
  for (int i; i < 10; ++i) {}
}
