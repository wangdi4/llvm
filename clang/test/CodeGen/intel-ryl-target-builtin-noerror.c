// INTEL_FEATURE_CPU_RYL
// REQUIRES: intel_feature_cpu_ryl

// REQUIRES: x86-registered-target
// RUN: %clang_cc1 %s -triple=x86_64-linux-gnu -S -o -
#define __MM_MALLOC_H
#include <x86intrin.h>

void verifycpustrings(void) {
  (void)__builtin_cpu_is("royal");
}

// end INTEL_FEATURE_CPU_RYL
