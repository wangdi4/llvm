// INTEL_FEATURE_ISA_PREFETCHI
// REQUIRES: intel_feature_isa_prefetchi
// RUN: %clang_cc1 -fsyntax-only -verify %s

void foo() {
  int a;
  __builtin_prefetch(&a);
  __builtin_prefetch(&a, 1);
  __builtin_prefetch(&a, 1, 2);
  __builtin_prefetch(&a, 1, 2, 0);
  __builtin_prefetch(&a, 1, 9, 8, 3); // expected-error{{too many arguments to function}}
}
// end INTEL_FEATURE_ISA_PREFETCHI
