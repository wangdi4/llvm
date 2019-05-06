// INTEL_FEATURE_CSA
// ^ copied from other CSA tests; see note below:
// TODO (vzakhari 11/14/2018): this is actually a hack that we do not use
//       #if guard above.  We have to live with this, until driver
//       starts defining the proper INTEL_FEATURE macros.
// RUN: %clang_cc1 -triple csa -fsyntax-only -verify %s
// REQUIRES: csa-registered-target

typedef float __m64f __attribute__((__vector_size__(8)));

int main(void) {
  int a;
  double d;
  __m64f v;

  // These tests are stolen from ../builtin-prefetch.c.
  __builtin_csa_gated_prefetch(a, &a);
  __builtin_csa_gated_prefetch(a, &a, 1);
  __builtin_csa_gated_prefetch(a, &a, 1, 2);
  __builtin_csa_gated_prefetch(a, &a, 1, 9, 3); // expected-error{{too many arguments to function}}
  __builtin_csa_gated_prefetch(a, &a, "hello", 2); // expected-error{{argument to '__builtin_csa_gated_prefetch' must be a constant integer}}
  __builtin_csa_gated_prefetch(a, &a, a, 2); // expected-error{{argument to '__builtin_csa_gated_prefetch' must be a constant integer}}
  __builtin_csa_gated_prefetch(a, &a, 2); // expected-error{{argument value 2 is outside the valid range [0, 1]}}
  __builtin_csa_gated_prefetch(a, &a, 0, 4); // expected-error{{argument value 4 is outside the valid range [0, 3]}}
  __builtin_csa_gated_prefetch(a, &a, -1, 4); // expected-error{{argument value -1 is outside the valid range [0, 1]}}

  // We also need to make sure the pointer argument is checked.
  __builtin_csa_gated_prefetch(a, a); // expected-error{{CSA builtin parameter must be a pointer}}
  __builtin_csa_gated_prefetch(a, d); // expected-error{{CSA builtin parameter must be a pointer}}
  __builtin_csa_gated_prefetch(a, v); // expected-error{{CSA builtin parameter must be a pointer}}

  // We should be able to use any non-aggregate type for the data.
  __builtin_csa_gated_prefetch(a, &a);
  __builtin_csa_gated_prefetch(d, &a);
  __builtin_csa_gated_prefetch(v, &a);
  __builtin_csa_gated_prefetch(&a, &a);

  // Also constants.
  __builtin_csa_gated_prefetch(0, &a);
}

// end INTEL_FEATURE_CSA
