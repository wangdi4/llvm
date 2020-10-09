// RUN: %clang_cc1 -verify -fintel-pragma-prefetch -fintel-compatibility %s
//
// Test semantic warnings related to #pragma [no]prefetch
void prefetch() {
  int A;
  int B;

// expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma prefetch'}}
#pragma prefetch
   A = B;
// expected-warning@+1 {{hint value, 0, must be between 1 and 4, inclusive}}
#pragma prefetch A:0
  for (int i; i < 10; ++i) {}
// expected-warning@+1 {{hint value, 5, must be between 1 and 4, inclusive}}
#pragma prefetch A:5
  for (int i; i < 10; ++i) {}
// expected-warning@+1 {{distance value must be greater than zero}}
#pragma prefetch A:4:0
  for (int i; i < 10; ++i) {}
}
