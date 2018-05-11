// RUN: %clang_cc1 -DFULL -fintel-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -DONLY -fintel-compatibility-enable=UnrollExtensions -fsyntax-only -verify %s
// RUN: %clang_cc1 -DDISABLE -fintel-compatibility -fintel-compatibility-disable=UnrollExtensions -fsyntax-only -verify %s
// RUN: %clang_cc1 -DOFF -fsyntax-only -verify %s

int main() {
  int i = 0, x = 0;
  #pragma unroll // OK, #pragma unroll precedes a loop
  for (i = 0; i < 20; ++i)
    ++x;

#if defined(FULL) || defined(ONLY)
  #pragma unroll (4) // OK in IntelCompat mode, #pragma unroll before statement.
  i = 0;
  while (i < 20) {
    ++x;
    ++i;
  }

  // Multiple #pragma unroll attributes are OK in IntelCompat mode.
  #pragma unroll
  #pragma unroll (4)
  #pragma nounroll
  #pragma unroll (8)
  // #pragma clang loop is not affected - must produce an error
  #pragma clang loop unroll_count (4) // expected-error{{duplicate directives '#pragma unroll(8)' and 'unroll_count(4)'}}
  for (i = 0; i < 20; ++i)
    ++x;
#endif
#if defined(DISABLE) || defined(OFF)
  // expected-error@+2{{expected a for, while, or do-while loop to follow}}
  #pragma unroll (4)
  i = 0;
  while (i < 20) {
    ++x;
    ++i;
  }
  #pragma unroll (4) // expected-error{{duplicate directives}}
  #pragma unroll (8)
  while (i < 20) {
    ++x;
    ++i;
  }
#endif

  #pragma clang loop unroll(enable)      // expected-error@+3{{expected a for, while, or do-while loop to follow}}
  #pragma clang loop interleave_count(4) // expected-error@+2{{expected a for, while, or do-while loop to follow}}
  #pragma clang loop vectorize(enable)   // expected-error@+1{{expected a for, while, or do-while loop to follow}}
  i = 0;
  for (i = 0; i < 20; ++i)
    ++x;
}
