// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

// This test has been changed to expect Intel pragmas use similiar rules to
// clang instead of the strange behavior icc exhibited.

// RUN: %clang_cc1 -DOFF -fsyntax-only -verify %s

int main() {
  int i = 0, x = 0;
  #pragma unroll // OK, #pragma unroll precedes a loop
  for (i = 0; i < 20; ++i)
    ++x;

  // expected-error@+2{{expected a for, while, or do-while loop to follow}}
  #pragma unroll (4)
  i = 0;
  while (i < 20) {
    ++x;
    ++i;
  }

  // expected-error@+2{{duplicate directives 'unroll_count(4)' and '#pragma unroll(8)'}}
  #pragma clang loop unroll_count (4)
  #pragma unroll(8)
  for (i = 0; i < 20; ++i)
    ++x;

  #pragma unroll (4)
  #pragma unroll (8) // expected-error{{duplicate directives}}
  while (i < 20) {
    ++x;
    ++i;
  }

  #pragma clang loop unroll(enable)      // expected-error@+3{{expected a for, while, or do-while loop to follow}}
  #pragma clang loop interleave_count(4) // expected-error@+2{{expected a for, while, or do-while loop to follow}}
  #pragma clang loop vectorize(enable)   // expected-error@+1{{expected a for, while, or do-while loop to follow}}
  i = 0;
  for (i = 0; i < 20; ++i)
    ++x;
}
