// RUN: %clang_cc1 -verify -fintel-pragma-prefetch -fintel-compatibility %s
//
// Test parsing warnings/errors related to #pragma [no]prefetch
void prefetch() {
  int A;
  int B;

// expected-warning@+1 {{extra tokens at end of '#pragma noprefetch'}}
#pragma noprefetch *
// expected-warning@+1 {{extra tokens at end of '#pragma noprefetch'}}
#pragma noprefetch *:
// expected-warning@+1 {{extra tokens at end of '#pragma noprefetch'}}
#pragma noprefetch 1:2
// expected-error@+1 {{expected ':'}}
#pragma prefetch *
// expected-error@+1 {{expected an integer argument in '#pragma prefetch'}}
#pragma prefetch *:
// expected-error@+1 {{expected an integer argument in '#pragma prefetch'}}
#pragma prefetch *:1:
// expected-error@+1 {{expected an integer argument in '#pragma prefetch'}}
#pragma prefetch A:1:
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch'}}
#pragma prefetch *:1, A
}
