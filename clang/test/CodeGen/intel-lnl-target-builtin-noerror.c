// INTEL_FEATURE_CPU_LNL
// REQUIRES: intel_feature_cpu_lnl

// REQUIRES: x86-registered-target
// RUN: %clang_cc1 %s -triple=x86_64-linux-gnu -S -o -
#define __MM_MALLOC_H
#include <x86intrin.h>

void verifycpustrings(void) {
  (void)__builtin_cpu_is("lunarlake");
}

// end INTEL_FEATURE_CPU_LNL
