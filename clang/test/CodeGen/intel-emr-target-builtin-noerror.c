// INTEL_FEATURE_CPU_EMR
// REQUIRES: intel_feature_cpu_emr

// REQUIRES: x86-registered-target
// RUN: %clang_cc1 %s -triple=x86_64-linux-gnu -S -o -
#define __MM_MALLOC_H
#include <x86intrin.h>

void verifycpustrings(void) {
  (void)__builtin_cpu_is("emeraldrapids");
}

// end INTEL_FEATURE_CPU_EMR
