// INTEL_CUSTOMIZATION
// INTEL_FEATURE_ISA_PREFETCHST2
// UNSUPPORTED: intel_feature_isa_prefetchst2
// end INTEL_FEATURE_ISA_PREFETCHST2
// end INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -fsyntax-only -verify %s

void foo(void) {
  int a;
  __builtin_prefetch(&a);
  __builtin_prefetch(&a, 1);
  __builtin_prefetch(&a, 1, 2);
  __builtin_prefetch(&a, 1, 9, 3); // expected-error{{too many arguments to function}}
  __builtin_prefetch(&a, "hello", 2); // expected-error{{argument to '__builtin_prefetch' must be a constant integer}}
  __builtin_prefetch(&a, a, 2); // expected-error{{argument to '__builtin_prefetch' must be a constant integer}}
  __builtin_prefetch(&a, 2); // expected-error{{argument value 2 is outside the valid range [0, 1]}}
  __builtin_prefetch(&a, 0, 4); // expected-error{{argument value 4 is outside the valid range [0, 3]}}
  __builtin_prefetch(&a, -1, 4); // expected-error{{argument value -1 is outside the valid range [0, 1]}}
}
