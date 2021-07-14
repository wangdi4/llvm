// REQUIRES: intel_feature_cpu_grt
// RUN: %clang_cc1 %s -triple=x86_64-linux-gnu -S -o -
#define __MM_MALLOC_H

#include <x86intrin.h>

void verifycpustrings() {
  (void)__builtin_cpu_is("gracemont");
}
